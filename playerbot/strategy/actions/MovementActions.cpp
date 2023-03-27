#include "botpch.h"
#include "../../playerbot.h"
#include "../values/LastMovementValue.h"
#include "MovementActions.h"
#include "MotionMaster.h"
#include "MovementGenerator.h"
#include "../../FleeManager.h"
#include "../../LootObjectStack.h"
#include "../../PlayerbotAIConfig.h"
#include "../../ServerFacade.h"
#include "../values/PositionValue.h"
#include "../values/Stances.h"
#include "MotionGenerators/TargetedMovementGenerator.h"
#include "../../TravelMgr.h"
#include "Entities/Transports.h"
#ifdef MANGOSBOT_TWO
#include "Entities/Vehicle.h"
#endif
#include "../generic/CombatStrategy.h"

using namespace ai;

void MovementAction::CreateWp(Player* wpOwner, float x, float y, float z, float o, uint32 entry, bool important)
{
    float dist = wpOwner->GetDistance(x, y, z);
    float delay = 1000.0f * dist / wpOwner->GetSpeed(MOVE_RUN) + sPlayerbotAIConfig.reactDelay;

    //if(!important)
    //    delay *= 0.25;

    Creature* wpCreature = wpOwner->SummonCreature(entry, x, y, z - 1, o, TEMPSPAWN_TIMED_DESPAWN, delay);
    ai->AddAura(wpCreature, 246);

    if (!important)
        wpCreature->SetObjectScale(0.5f);
}

bool MovementAction::isPossible()
{
    // Do not move if stay strategy is set
    if (ai->HasStrategy("stay", ai->GetState()))
        return false;

    return true;
}

bool MovementAction::MoveNear(uint32 mapId, float x, float y, float z, float distance)
{
    float angle = GetFollowAngle();
    return MoveTo(mapId, x + cos(angle) * distance, y + sin(angle) * distance, z);
}

bool MovementAction::MoveNear(WorldObject* target, float distance)
{
    if (!target)
        return false;

#ifdef MANGOS
    distance += target->GetObjectBoundingRadius();
#endif

    float x = target->GetPositionX();
    float y = target->GetPositionY();
    float z = target->GetPositionZ();
    float followAngle = GetFollowAngle();
    for (float angle = followAngle; angle <= followAngle + 2 * M_PI; angle += M_PI_F / 4.0f)
    {
#ifdef CMANGOS
        float dist = distance + target->GetObjectBoundingRadius();
        target->GetNearPoint(bot, x, y, z, bot->GetObjectBoundingRadius(), min(dist, ai->GetRange("follow")), angle);
#endif
#ifdef MANGOS
        float x = target->GetPositionX() + cos(angle) * distance,
             y = target->GetPositionY()+ sin(angle) * distance,
             z = target->GetPositionZ();
#endif
        if (!bot->IsWithinLOS(x, y, z + bot->GetCollisionHeight(), true))
            continue;
        bool moved = MoveTo(target->GetMapId(), x, y, z);
        if (moved)
            return true;
    }

    //ai->TellError("All paths not in LOS");
    return false;
}

bool MovementAction::MoveToLOS(WorldObject* target, bool ranged)
{
    if (!target)
        return false;

    //ostringstream out; out << "Moving to LOS!";
    //bot->Say(out.str(), LANG_UNIVERSAL);

    float x = target->GetPositionX();
    float y = target->GetPositionY();
    float z = target->GetPositionZ();

    //Use standard pathfinder to find a route. 
    PathFinder path(bot);
    path.calculate(x, y, z, false);
    PathType type = path.getPathType();
    if (type != PATHFIND_NORMAL && type != PATHFIND_INCOMPLETE)
        return false;

    if (!ranged)
        return MoveTo((Unit*)target, target->GetObjectBoundingRadius());

    float dist = FLT_MAX;
    PositionEntry dest;

    if (!path.getPath().empty())
    {
        for (auto& point : path.getPath())
        {
            if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                CreateWp(bot, point.x, point.y, point.z, 0.0, 2334);

            float distPoint = sqrt(target->GetDistance(point.x, point.y, point.z, DIST_CALC_NONE));
            if (distPoint < dist && target->IsWithinLOS(point.x, point.y, point.z + bot->GetCollisionHeight()))
            {
                dist = distPoint;
                dest.Set(point.x, point.y, point.z, target->GetMapId());

                if (ranged)
                    break;
            }
        }
    }

    if (dest.isSet())
        return MoveTo(dest.mapId, dest.x, dest.y, dest.z);
    else
        ai->TellError("All paths not in LOS");

    return false;
}

bool MovementAction::FlyDirect(WorldPosition &startPosition, WorldPosition &endPosition, WorldPosition& movePosition, TravelPath movePath, bool idle)
{
    //Fly directly.
#ifdef MANGOSBOT_ZERO
    return false;
#else
    if (!bot->IsFreeFlying())
        return false;

    if (!startPosition.isOutside())
        return false;

    float totalDistance = startPosition.distance(endPosition);  //Total distance to where we want to go
    float minDist = sPlayerbotAIConfig.targetPosRecalcDistance; //Minium distance a bot should move.
    float maxDist = sPlayerbotAIConfig.reactDistance;           //Maxium distance a bot can move in one single action.

    if (totalDistance < maxDist && !bot->IsFlying())
        return false;

    movePosition = endPosition;

    if (movePosition.getMapId() != startPosition.getMapId() || !movePosition.isOutside() || !movePosition.canFly()) //We can not fly to the end directly.
    {
        vector<WorldPosition> path;
        if (movePath.empty()) //Make a path starting at the end backwards to see if we can walk to some better place.
        {
            path = endPosition.getPathTo(startPosition, bot);
            std::reverse(path.begin(), path.end());
        }
        else
            path = movePath.getPointPath();

        if (path.empty())
            return false;

        auto pathEnd = path.end();
        for (auto& p = pathEnd; p-- != path.begin(); ) //Find the furtest point where we can fly to directly.
            if (p->getMapId() == startPosition.getMapId() && p->isOutside() && p->canFly())
            {
                movePosition = *p;
                totalDistance = startPosition.distance(movePosition);
                break;
            }
    }

    if (movePosition.getMapId() != startPosition.getMapId() || !movePosition.isOutside() || !movePosition.canFly())
        return false;

    if (movePosition.distance(startPosition) < minDist)
    {
        movePath.clear();
        AI_VALUE(LastMovement&, "last movement").setPath(movePath);

        if (movePosition.currentHeight() < minDist)
            return false;
        else
            movePosition.setZ(movePosition.getHeight());
    }

    uint32 flyHeight = 0;
    float originalZ = endPosition.getZ();
    bool detailedMove = ai->AllowActivity(DETAILED_MOVE_ACTIVITY);

    //Crop the distance we can travel to maxDist;
    if (totalDistance > maxDist)
    {
        flyHeight = std::min(100.0f, totalDistance / 10.0f);

        movePosition = movePosition.limit(startPosition, maxDist);

        if (!bot->IsFlying())
        {
            WorldPacket data(SMSG_SPLINE_MOVE_SET_FLYING, 9);
            data << bot->GetPackGUID();
            bot->SendMessageToSet(data, true);

            if (!bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FLYING))
                bot->m_movementInfo.AddMovementFlag(MOVEFLAG_FLYING);
#ifdef MANGOSBOT_ONE
            if (!bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FLYING2))
                bot->m_movementInfo.AddMovementFlag(MOVEFLAG_FLYING2);
#endif
            if (!bot->m_movementInfo.HasMovementFlag(MOVEFLAG_LEVITATING))
                bot->m_movementInfo.AddMovementFlag(MOVEFLAG_LEVITATING);
        }
    }
    else
    {
        bool needLand = false;

        if (const TerrainInfo* terrain = bot->GetTerrain())
        {
            float height = terrain->GetHeightStatic(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
            float ground = terrain->GetWaterOrGroundLevel(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), height);
            if (bot->GetPositionZ() > originalZ && (bot->GetPositionZ() - originalZ < 5.0f) && (fabs(originalZ - ground) < 5.0f))
                needLand = true;
        }
        if (needLand)
        {
            WorldPacket data(SMSG_SPLINE_MOVE_UNSET_FLYING, 9);
            data << bot->GetPackGUID();
            bot->SendMessageToSet(data, true);

            if (bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FLYING))
                bot->m_movementInfo.RemoveMovementFlag(MOVEFLAG_FLYING);
#ifdef MANGOSBOT_ONE
            if (bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FLYING2))
                bot->m_movementInfo.RemoveMovementFlag(MOVEFLAG_FLYING2);
#endif
            if (bot->m_movementInfo.HasMovementFlag(MOVEFLAG_LEVITATING))
                bot->m_movementInfo.RemoveMovementFlag(MOVEFLAG_LEVITATING);
        }
    }

    for (uint32 modZ = 0; modZ < maxDist / 5.0f; modZ++)
    {
        if (movePosition.currentHeight() > flyHeight && startPosition.IsInLineOfSight(movePosition))
            break;

        movePosition.setZ(movePosition.getZ() + 5.0f);

        if (movePosition.distance(startPosition) > maxDist)
            movePosition = movePosition.limit(startPosition, maxDist);
    }

    LastMovement& lastMove = AI_VALUE(LastMovement&,"last movement");
    if (sPlayerbotAIConfig.hasLog("bot_movement.csv") && lastMove.lastMoveShort != movePosition)
    {
        ostringstream out;
        out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
        out << bot->GetName() << ",";
        startPosition.printWKT({ startPosition, movePosition }, out, 1);
        out << to_string(bot->getRace()) << ",";
        out << to_string(bot->getClass()) << ",";
        float subLevel = ((float)bot->GetLevel() + ((float)bot->GetUInt32Value(PLAYER_XP) / (float)bot->GetUInt32Value(PLAYER_NEXT_LEVEL_XP)));
        out << subLevel << ",";
        out << "1";

        sPlayerbotAIConfig.log("bot_movement.csv", out.str().c_str());
    }

    if (totalDistance > maxDist && !detailedMove && !ai->HasPlayerNearby(&movePosition)) //Why walk if you can fly?
    {
        time_t now = time(0);

        AI_VALUE(LastMovement&, "last movement").nextTeleport = now + (time_t)MoveDelay(startPosition.distance(movePosition));

        return bot->TeleportTo(movePosition.getMapId(), movePosition.getX(), movePosition.getY(), movePosition.getZ(), startPosition.getAngleTo(movePosition));
    }

    MotionMaster& mm = *bot->GetMotionMaster();

    //Clean movement if not already moving the same way.
    if (mm.GetCurrent()->GetMovementGeneratorType() != POINT_MOTION_TYPE)
    {
        ai->StopMoving();
        mm.Clear();
    }
    else
    {
        float x, y, z;
        mm.GetDestination(x, y, z);

        if (movePosition.distance(WorldPosition(movePosition.getMapId(), x, y, z, 0)) > minDist)
        {
            ai->StopMoving();
            mm.Clear();
        }
    }
    mm.Clear(false, true);
    mm.MovePoint(movePosition.getMapId(), Position(movePosition.getX(), movePosition.getY(), movePosition.getZ(), 0.f), bot->IsFlying() ? FORCED_MOVEMENT_FLIGHT : FORCED_MOVEMENT_RUN, bot->IsFlying() ? bot->GetSpeed(MOVE_FLIGHT) : 0.f, bot->IsFlying());

    AI_VALUE(LastMovement&, "last movement").setShort(startPosition, movePosition);

    if (!idle)
        ClearIdleState();

    return true;
#endif
}

bool MovementAction::MoveTo(uint32 mapId, float x, float y, float z, bool idle, bool react, bool noPath, bool ignoreEnemyTargets)
{
    WorldPosition endPosition(mapId, x, y, z, 0);
    if(!endPosition.isValid())
        return false;

    UpdateMovementState();

    if (!IsMovingAllowed())
        return false;

    bool isVehicle = false;
    Unit* mover = bot;
#ifdef MANGOSBOT_TWO
    TransportInfo* transportInfo = bot->GetTransportInfo();
    if (transportInfo && transportInfo->IsOnVehicle())
    {
        Unit* vehicle = (Unit*)transportInfo->GetTransport();
        if (vehicle && vehicle->GetVehicleInfo())
        {
            VehicleSeatEntry const* seat = vehicle->GetVehicleInfo()->GetSeatEntry(transportInfo->GetTransportSeat());
            if (!seat || !seat->HasFlag(SEAT_FLAG_CAN_CONTROL))
                return false;
        }
        
        isVehicle = true;
        mover = vehicle;
    }
#endif

    bool detailedMove = ai->AllowActivity(DETAILED_MOVE_ACTIVITY,true);

    if (!detailedMove)
    {
        time_t now = time(0);
        if (AI_VALUE(LastMovement&, "last movement").nextTeleport > now) //We can not teleport yet. Wait.
        {
            SetDuration((AI_VALUE(LastMovement&, "last movement").nextTeleport - now) * 1000);
            return true;
        }
    }

    float minDist = sPlayerbotAIConfig.targetPosRecalcDistance; //Minimum distance a bot should move.
    float maxDist = sPlayerbotAIConfig.sightDistance;           //Maximum distance a bot can move in one single action.
    float originalZ = z;                                        // save original destination height to check if bot needs to fly up


    bool generatePath = !bot->IsFlying() && !bot->HasMovementFlag(MOVEFLAG_SWIMMING) && !bot->IsInWater() && !sServerFacade.IsUnderwater(bot);
    if (noPath)
        generatePath = false;

    if (!isVehicle && !IsMovingAllowed() && sServerFacade.UnitIsDead(bot))
    {
        return false;
    }

    if (!isVehicle && bot->IsMoving() && !IsMovingAllowed())
    {
        return false;
    }

    LastMovement& lastMove = *context->GetValue<LastMovement&>("last movement");

    WorldPosition startPosition = WorldPosition(bot);             //Current location of the bot
    WorldPosition movePosition;

    float totalDistance = startPosition.distance(endPosition);    //Total distance to where we want to go
    float maxDistChange = totalDistance * 0.1;                    //Maximum change between previous destination before needing a recalculation
    TravelPath movePath;

    if (totalDistance < minDist)
    {
        if (lastMove.lastMoveShort.distance(endPosition) < maxDistChange)
            AI_VALUE(LastMovement&, "last movement").clear();
        if (mover == bot)
            ai->StopMoving();
        else
            mover->InterruptMoving(true);
        return false;
    }

    bool isWalking = false;

    if (FlyDirect(startPosition, endPosition, movePosition, lastMove.lastPath, idle)) //Try flying in a straight line to target.
        return true;

    if (lastMove.lastMoveShort.distance(endPosition) < maxDistChange && startPosition.distance(lastMove.lastMoveShort) < maxDist) //The last short movement was to the same place we want to move now.
        movePosition = endPosition;
    else if (!lastMove.lastPath.empty() && lastMove.lastPath.getBack().distance(endPosition) < maxDistChange) //The last long movement was to the same place we want to move now.
    {
        movePath = lastMove.lastPath;
    }
    else
    {
        movePosition = endPosition;

        if (startPosition.getMapId() != endPosition.getMapId() || totalDistance > maxDist)
        {
            if (!sTravelNodeMap.getNodes().empty() && !bot->InBattleGround())
            {
                if (sPlayerbotAIConfig.tweakValue)
                {
                    if (lastMove.future.valid())
                    {
                        if(lastMove.future.wait_for(0s) != future_status::ready)
                            return true; //we have no path. Wait until it is ready.
                        movePath = lastMove.future.get();
                    }
                    else
                    {
                        lastMove.future = std::async(&TravelNodeMap::getFullPath, startPosition, endPosition, bot);
                        return true;
                    }
                }
                else
                    movePath = sTravelNodeMap.getFullPath(startPosition, endPosition, bot);

                if (movePath.empty())
                {
                    //We have no path. Beyond 450yd the standard pathfinder will probably move the wrong way.
                    if (sServerFacade.IsDistanceGreaterThan(totalDistance, maxDist * 3))
                    {
                        movePath.clear();
                        movePath.addPoint(endPosition);
                        AI_VALUE(LastMovement&, "last movement").setPath(movePath);

                        if (mover == bot)
                            ai->StopMoving();
                        else
                            mover->InterruptMoving(true);
                        if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                            ai->TellMasterNoFacing("I have no path");
                        return false;
                    }

                    movePosition = endPosition;
                }
            }
            else
            {
                //Use standard pathfinder to find a route. 
                movePosition = endPosition;
            }
        }
    }

    if (movePath.empty() && movePosition.distance(startPosition) > maxDist)
    {
        //Use standard pathfinder to find a route. 
        PathFinder path(mover);
        path.calculate(movePosition.getX(), movePosition.getY(), movePosition.getZ(), false);
        PathType type = path.getPathType();
        PointsArray& points = path.getPath();
        movePath.addPath(startPosition.fromPointsArray(points));
    }

    if (!lastMove.lastPath.empty() && !movePath.empty() && lastMove.lastPath.getBack().distance(endPosition) <= movePath.getBack().distance(endPosition)) //new path is worse than the last path. Keep going the old path.
    {
        movePath = lastMove.lastPath;
    }

    if (!movePath.empty())
    {
        if (movePath.makeShortCut(startPosition, maxDist,bot))
            if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                ai->TellMasterNoFacing("Found a shortcut.");

        if (movePath.empty())
        {

            AI_VALUE(LastMovement&, "last movement").setPath(movePath);

            if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                ai->TellMasterNoFacing("Too far from path. Rebuilding.");
            return true;
        }

        TravelNodePathType pathType;
        uint32 entry;
        movePosition = movePath.getNextPoint(startPosition, maxDist, pathType, entry);

        if (pathType == TravelNodePathType::staticPortal && entry)// && !ai->isRealPlayer())
        {
            //Log bot movement
            if (sPlayerbotAIConfig.hasLog("bot_movement.csv"))
            {
                WorldPosition telePos;
                if (entry)
                {
                    AreaTrigger const* at = sObjectMgr.GetAreaTrigger(entry);
                    if (at)
                        telePos = WorldPosition(at->target_mapId, at->target_X, at->target_Y, at->target_Z, at->target_Orientation);
                }
                else
                    telePos = movePosition;

                ostringstream out;
                out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
                out << bot->GetName() << ",";
                if (telePos && telePos != movePosition)
                    startPosition.printWKT({ startPosition, movePosition, telePos }, out, 1);
                else
                    startPosition.printWKT({ startPosition, movePosition }, out, 1);

                out << to_string(bot->getRace()) << ",";
                out << to_string(bot->getClass()) << ",";
                float subLevel = ((float)bot->GetLevel() + ((float)bot->GetUInt32Value(PLAYER_XP) / (float)bot->GetUInt32Value(PLAYER_NEXT_LEVEL_XP)));
                out << subLevel << ",";
                out << (entry ? entry : -1);

                sPlayerbotAIConfig.log("bot_movement.csv", out.str().c_str());
            }

            GameObjectInfo const* goInfo = sGOStorage.LookupEntry<GameObjectInfo>(entry);
            if (!goInfo || goInfo->type != GAMEOBJECT_TYPE_SPELLCASTER)
                return false;

            uint32 spellId = goInfo->spellcaster.spellId;
            const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);

            if (pSpellInfo->EffectTriggerSpell[0])
                pSpellInfo = sServerFacade.LookupSpellInfo(pSpellInfo->EffectTriggerSpell[0]);

            if (pSpellInfo->Effect[0] != SPELL_EFFECT_TELEPORT_UNITS && pSpellInfo->Effect[1] != SPELL_EFFECT_TELEPORT_UNITS && pSpellInfo->Effect[2] != SPELL_EFFECT_TELEPORT_UNITS)
                return false;

            if (bot->IsMounted())
            {
                if (bot->IsFlying() && WorldPosition(bot).currentHeight() > 10.0f)
                    return false;

                WorldPacket emptyPacket;
                bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);

                if (bot->IsFlying())
                    bot->GetMotionMaster()->MoveFall();
            }

            Spell* spell = new Spell(bot, pSpellInfo, false);
            SpellCastTargets targets;
            targets.setUnitTarget(bot);
#ifdef MANGOS
            spell->prepare(&targets, NULL);
#endif
#ifdef CMANGOS
            spell->SpellStart(&targets, NULL);
#endif
            SpellCastResult castResult = spell->cast(true);

            return castResult == SPELL_CAST_OK;
        }

        if (pathType == TravelNodePathType::areaTrigger)// && !ai->isRealPlayer())
        {
            //Log bot movement
            if (sPlayerbotAIConfig.hasLog("bot_movement.csv"))
            {
                WorldPosition telePos;
                if (entry)
                {
                    AreaTrigger const* at = sObjectMgr.GetAreaTrigger(entry);
                    if (at)
                        telePos = WorldPosition(at->target_mapId, at->target_X, at->target_Y, at->target_Z, at->target_Orientation);
                }
                else
                    telePos = movePosition;

                ostringstream out;
                out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
                out << bot->GetName() << ",";
                if (telePos && telePos != movePosition)
                    startPosition.printWKT({ startPosition, movePosition, telePos }, out, 1);
                else
                    startPosition.printWKT({ startPosition, movePosition }, out, 1);

                out << to_string(bot->getRace()) << ",";
                out << to_string(bot->getClass()) << ",";
                float subLevel = ((float)bot->GetLevel() + ((float)bot->GetUInt32Value(PLAYER_XP) / (float)bot->GetUInt32Value(PLAYER_NEXT_LEVEL_XP)));
                out << subLevel << ",";
                out << (entry ? entry : -1);

                sPlayerbotAIConfig.log("bot_movement.csv", out.str().c_str());
            }

            if (entry)
            {
                AI_VALUE(LastMovement&, "last area trigger").lastAreaTrigger = entry;
            }
            else
                return bot->TeleportTo(movePosition.getMapId(), movePosition.getX(), movePosition.getY(), movePosition.getZ(), movePosition.getO(), 0);
        }

        if (pathType == TravelNodePathType::transport && entry)
        {
            if (!bot->GetTransport())
            {
                for (auto& transport : movePosition.getTransports(entry))
                    if (movePosition.sqDistance2d(WorldPosition((WorldObject*)transport)) < 5 * 5)
                        transport->AddPassenger(bot, true);
            }
            WaitForReach(100.0f);
            return true;
        }

        if (pathType == TravelNodePathType::flightPath && entry)
        {
            TaxiPathEntry const* tEntry = sTaxiPathStore.LookupEntry(entry);

            if (tEntry)
            {
                Creature* unit = nullptr;

                if (!bot->m_taxi.IsTaximaskNodeKnown(tEntry->from))
                {
                    list<ObjectGuid> npcs = AI_VALUE(list<ObjectGuid>, "nearest npcs");
                    for (list<ObjectGuid>::iterator i = npcs.begin(); i != npcs.end(); i++)
                    {
                        Creature* unit = bot->GetNPCIfCanInteractWith(*i, UNIT_NPC_FLAG_FLIGHTMASTER);
                        if (!unit)
                            continue;

                        bot->GetSession()->SendLearnNewTaxiNode(unit);

                        unit->SetFacingTo(unit->GetAngle(bot));
                    }
                }

                uint32 botMoney = bot->GetMoney();
                if (ai->HasCheat(BotCheatMask::gold) || ai->HasCheat(BotCheatMask::taxi))
                {
                    bot->SetMoney(10000000);
                }

                bool goTaxi = bot->ActivateTaxiPathTo({ tEntry->from, tEntry->to }, unit, 1);

                if (ai->HasCheat(BotCheatMask::gold) || ai->HasCheat(BotCheatMask::taxi))
                {
                    bot->SetMoney(botMoney);
                }

                return goTaxi;
            }
        }

        if (pathType == TravelNodePathType::teleportSpell && entry)
        { 
            if (entry == 8690)
            {
                if (sServerFacade.IsSpellReady(bot, 8690) && (!bot->IsFlying() || WorldPosition(bot).currentHeight() < 10.0f))
                {
                    return ai->DoSpecificAction("hearthstone", Event("move action"), true);

                }
                else
                {
                    movePath.clear();
                    AI_VALUE(LastMovement&, "last movement").setPath(movePath);
                    return false;
                }
            }
        }

        if (pathType == TravelNodePathType::walk && movePath.getPath().begin()->type != PathNodeType::NODE_FLIGHTPATH)
            isWalking = true;
        //if (!isTransport && bot->GetTransport())
        //    bot->GetTransport()->RemovePassenger(bot);
    }

    AI_VALUE(LastMovement&, "last movement").setPath(movePath);

    if (!movePosition || movePosition.getMapId() != bot->GetMapId())
    {
        movePath.clear();
        AI_VALUE(LastMovement&, "last movement").setPath(movePath);

        if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
            ai->TellMasterNoFacing("No point. Rebuilding.");
        return false;
    }

    if (movePosition.distance(startPosition) > maxDist)
    {
        //Use standard pathfinder to find a route. 
        PathFinder path(mover);
        path.calculate(movePosition.getX(), movePosition.getY(), movePosition.getZ(), false);
        PathType type = path.getPathType();
        PointsArray& points = path.getPath();
        movePath.addPath(startPosition.fromPointsArray(points));
        TravelNodePathType pathType;
        uint32 entry;
        movePosition = movePath.getNextPoint(startPosition, maxDist, pathType, entry);
    }

    //Stop the path when we might get aggro.
    if (!bot->IsInCombat() && !bot->IsDead() && !ignoreEnemyTargets) 
    {
        list<ObjectGuid> targets = AI_VALUE_LAZY(list<ObjectGuid>, "possible attack targets");

        if (!targets.empty() && movePosition)
        {
            PathFinder path(mover);
            path.calculate(movePosition.getX(), movePosition.getY(), movePosition.getZ(), false);
            PathType type = path.getPathType();
            PointsArray& points = path.getPath();
            bool foundAggro = false;

            for (auto p : points)
            {
                WorldPosition point(startPosition.getMapId(), p.x, p.y, p.z, startPosition.getO());
                for (auto target : targets)
                {
                    if (!target.IsCreature())
                        continue;

                    Unit* unit = ai->GetUnit(target);
                    if (!unit)
                        continue;

                    if (unit->IsDead())
                        continue;

                    float range = unit->GetAttackDistance(bot);

                    if (WorldPosition(unit).sqDistance(point) > range * range)
                        continue;

                    if (!unit->CanAttackOnSight(bot))
                        continue;

                    if (!unit->IsWithinLOSInMap(bot))
                        continue;

                    if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                        ai->TellMasterNoFacing("Found " + chat->formatWorldobject(unit) + " stopping early.");

                    movePosition = point;
                    foundAggro = true;
                    break;
                }
                if (foundAggro)
                    break;
            }
        }
    }


    if (movePosition == WorldPosition()) {
        movePath.clear();
        AI_VALUE(LastMovement&, "last movement").setPath(movePath);

        if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
            ai->TellMasterNoFacing("No point. Rebuilding.");
        return false;
    }

    //Visual waypoints
    if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
    {
        if (!movePath.empty())
        {
            float cx = x;
            float cy = y;
            float cz = z;
            for (auto i : movePath.getPath())
            {
                CreateWp(bot, i.point.getX(), i.point.getY(), i.point.getZ(), 0.0, 2334);

                cx = i.point.getX();
                cy = i.point.getY();
                cz = i.point.getZ();
            }
        }
        else
            CreateWp(bot, movePosition.getX(), movePosition.getY(), movePosition.getZ(), 0, 2334, true);
    }

    //Log bot movement
    if (sPlayerbotAIConfig.hasLog("bot_movement.csv") && lastMove.lastMoveShort != movePosition)
    {
        ostringstream out;
        out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
        out << bot->GetName() << ",";
        startPosition.printWKT({ startPosition, movePosition }, out, 1);
        out << to_string(bot->getRace()) << ",";
        out << to_string(bot->getClass()) << ",";
        float subLevel = ((float)bot->GetLevel() + ((float)bot->GetUInt32Value(PLAYER_XP) / (float)bot->GetUInt32Value(PLAYER_NEXT_LEVEL_XP)));
        out << subLevel << ",";
        out << 0;

        sPlayerbotAIConfig.log("bot_movement.csv", out.str().c_str());
    }

    if (!react)
        if (totalDistance > maxDist)
            WaitForReach(startPosition.distance(movePosition) - 10.0f);
        else
            WaitForReach(startPosition.distance(movePosition));

    if (!isVehicle)
    {
        bot->HandleEmoteState(0);
        if (!bot->IsStandState())
            bot->SetStandState(UNIT_STAND_STATE_STAND);

        if (bot->IsNonMeleeSpellCasted(true))
        {
            bot->CastStop();
            ai->InterruptSpell();
        }
    }

    MotionMaster& mm = *mover->GetMotionMaster();

    /* Why do we do this?
    if (lastMove.lastMoveShort.distance(movePosition) < minDist)
    {
        bot->StopMoving();
        mm.Clear();
    }
    */

    // Clean movement if not already moving the same way.
    if (mm.GetCurrent()->GetMovementGeneratorType() != POINT_MOTION_TYPE)
    {
        if (mover == bot)
            ai->StopMoving();
        else if (mover)
            mover->InterruptMoving(true);
    }
    else
    {
        mm.GetDestination(x, y, z);

        if (movePosition.distance(WorldPosition(movePosition.getMapId(), x, y, z, 0)) > minDist)
        {
            if (mover == bot)
                ai->StopMoving();
            else
                mover->InterruptMoving(true);
        }
    }

    if (totalDistance > maxDist && !detailedMove && !ai->HasPlayerNearby(&movePosition)) //Why walk if you can fly?
    {
        time_t now = time(0);

        AI_VALUE(LastMovement&, "last movement").nextTeleport = now + (time_t)MoveDelay(startPosition.distance(movePosition));

        return bot->TeleportTo(movePosition.getMapId(), movePosition.getX(), movePosition.getY(), movePosition.getZ(), startPosition.getAngleTo(movePosition));
    }

    // walk if master walks and is close
    bool masterWalking = false;
    Unit* master = ai->GetMaster();
    if (master && sServerFacade.IsFriendlyTo(bot, master))
    {
        if (master->m_movementInfo.HasMovementFlag(MOVEFLAG_WALK_MODE) && sServerFacade.GetDistance2d(bot, master) < 20.0f)
            masterWalking = true;
    }

    bot->SendHeartBeat();

    // Prevent moving if requested to move into a hazard
    if(IsHazardNearPosition(movePosition))
    {
        if(!react)
        {
            SetDuration(sPlayerbotAIConfig.reactDelay);
        }

        return false;
    }

#ifdef MANGOSBOT_ZERO
    mm.Clear(false, true);

    Movement::PointsArray path;
    if(GeneratePathAvoidingHazards(movePosition, generatePath, path))
    {
        mm.MovePath(path, masterWalking ? FORCED_MOVEMENT_WALK : FORCED_MOVEMENT_RUN, false, false);
        WaitForReach(path);
    }
    else
    {
        mm.MovePoint(movePosition.getMapId(), movePosition.getX(), movePosition.getY(), movePosition.getZ(), masterWalking ? FORCED_MOVEMENT_WALK : FORCED_MOVEMENT_RUN, generatePath);
    }
    
#else
    if (!bot->IsFreeFlying())
    {
        mm.Clear(false, true);

        Movement::PointsArray path;
        if (GeneratePathAvoidingHazards(movePosition, generatePath, path))
        {
            mm.MovePath(path, masterWalking ? FORCED_MOVEMENT_WALK : FORCED_MOVEMENT_RUN, false, false);
            WaitForReach(path);
        }
        else
        {
            mm.MovePoint(movePosition.getMapId(), movePosition.getX(), movePosition.getY(), movePosition.getZ(), masterWalking ? FORCED_MOVEMENT_WALK : FORCED_MOVEMENT_RUN, generatePath);
        }
    }
    else
    {
        bool needFly = false;
        bool needLand = false;
        bool isFly = bot->IsFlying();
        bool isFar = false;

        // if bot is on flying mount, fly up or down depending on distance to target
        if (totalDistance > maxDist && isWalking)
        {
            isFar = true;
            needFly = true;
            // only use in clear LOS betweek points
            Position pos = bot->GetPosition();
#ifdef MANGOSBOT_TWO
            if (!bot->GetMap()->IsInLineOfSight(pos.x, pos.y, pos.z + 100.f, movePosition.getX(), movePosition.getY(), movePosition.getZ() + 100.f, bot->GetPhaseMask(), true))
#else
            if (!bot->GetMap()->IsInLineOfSight(pos.x, pos.y, pos.z + 100.f, movePosition.getX(), movePosition.getY(), movePosition.getZ() + 100.f, true))
#endif
                needFly = false;

            if (const TerrainInfo* terrain = bot->GetTerrain())
            {
                if (needFly)
                {
                    // get ground level data at next waypoint
                    float height = terrain->GetHeightStatic(movePosition.getX(), movePosition.getY(), movePosition.getZ());
                    float ground = terrain->GetWaterOrGroundLevel(movePosition.getX(), movePosition.getY(), movePosition.getZ(), height);

                    float botheight = terrain->GetHeightStatic(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
                    float botground = terrain->GetWaterOrGroundLevel(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), botheight);

                    // fly up if destination is far
                    if (totalDistance > maxDist && ground <= movePosition.getZ()) // check if ground level is not higher than path (tunnels)
                    {
                        movePosition.setZ(min(max(ground, botground) + 100.0f, max(movePosition.getZ() + 10.0f, bot->GetPositionZ() + 10.0f)));
                    }
                    else
                    {
                        movePosition.setZ(max(max(ground, botground), bot->GetPositionZ() - 10.0f));
                    }
                }
            }
        }

        if (!isFar && !isFly && originalZ > bot->GetPositionZ() && (originalZ - bot->GetPositionZ()) > 5.0f)
            needFly = true;

        if (needFly && !isFly)
        {
            WorldPacket data(SMSG_SPLINE_MOVE_SET_FLYING, 9);
            data << bot->GetPackGUID();
            bot->SendMessageToSet(data, true);

            if (!bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FLYING))
                bot->m_movementInfo.AddMovementFlag(MOVEFLAG_FLYING);
#ifdef MANGOSBOT_ONE
            if (!bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FLYING2))
                bot->m_movementInfo.AddMovementFlag(MOVEFLAG_FLYING2);
#endif
            if (!bot->m_movementInfo.HasMovementFlag(MOVEFLAG_LEVITATING))
                bot->m_movementInfo.AddMovementFlag(MOVEFLAG_LEVITATING);
        }

        if (!isFar && isFly)
        {
            if (const TerrainInfo* terrain = bot->GetTerrain())
            {
                float height = terrain->GetHeightStatic(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
                float ground = terrain->GetWaterOrGroundLevel(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), height);
                if (bot->GetPositionZ() > originalZ && (bot->GetPositionZ() - originalZ < 5.0f) && (fabs(originalZ - ground) < 5.0f))
                    needLand = true;
            }
            if (needLand)
            {
                WorldPacket data(SMSG_SPLINE_MOVE_UNSET_FLYING, 9);
                data << bot->GetPackGUID();
                bot->SendMessageToSet(data, true);

                if (bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FLYING))
                    bot->m_movementInfo.RemoveMovementFlag(MOVEFLAG_FLYING);
#ifdef MANGOSBOT_ONE
                if (bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FLYING2))
                    bot->m_movementInfo.RemoveMovementFlag(MOVEFLAG_FLYING2);
#endif
                if (bot->m_movementInfo.HasMovementFlag(MOVEFLAG_LEVITATING))
                    bot->m_movementInfo.RemoveMovementFlag(MOVEFLAG_LEVITATING);
            }
        }
        mm.Clear(false, true);
        mm.MovePoint(movePosition.getMapId(), Position(movePosition.getX(), movePosition.getY(), movePosition.getZ(), 0.f), bot->IsFlying() ? FORCED_MOVEMENT_FLIGHT : FORCED_MOVEMENT_RUN, bot->IsFlying() ? bot->GetSpeed(MOVE_FLIGHT) : 0.f, bot->IsFlying());
    }
#endif

    AI_VALUE(LastMovement&, "last movement").setShort(startPosition, movePosition);

    if (!idle)
        ClearIdleState();

    return true;
}

bool MovementAction::MoveTo(Unit* target, float distance)
{
    if (!target || !target->IsInWorld())
    {
        //ai->TellError("Seems I am stuck");
        return false;
    }

    float bx = bot->GetPositionX(), by = bot->GetPositionY(), bz = bot->GetPositionZ();
    float tx = target->GetPositionX(), ty = target->GetPositionY(), tz = target->GetPositionZ();

    if (sServerFacade.IsHostileTo(bot, target))
    {
        Stance* stance = AI_VALUE(Stance*, "stance");
        WorldLocation loc = stance->GetLocation();
        if (Formation::IsNullLocation(loc) || loc.mapid == -1)
        {
            //ai->TellError("Nowhere to move");
            return false;
        }

        tx = loc.coord_x;
        ty = loc.coord_y;
        tz = loc.coord_z;
    }

    float distanceToTarget = sServerFacade.GetDistance2d(bot, tx, ty);
    if (sServerFacade.IsDistanceGreaterThan(distanceToTarget, sPlayerbotAIConfig.targetPosRecalcDistance))
    {
        /*
        float angle = bot->GetAngle(tx, ty);
        float needToGo = distanceToTarget - distance;

        float maxDistance = ai->GetRange("spell");
        if (needToGo > 0 && needToGo > maxDistance)
            needToGo = maxDistance;
        else if (needToGo < 0 && needToGo < -maxDistance)
            needToGo = -maxDistance;

        float dx = cos(angle) * needToGo + bx;
        float dy = sin(angle) * needToGo + by;
        float dz = bz + (tz - bz) * needToGo / distanceToTarget;
        */

        float dx = tx;
        float dy = ty;
        float dz = tz;
        return MoveTo(target->GetMapId(), dx, dy, dz);
    }

    return true;
}

float MovementAction::GetFollowAngle()
{
    Player* master = GetMaster();
    Group* group = master ? master->GetGroup() : bot->GetGroup();
    if (!group)
        return 0.0f;

    int index = 1;
    for (GroupReference *ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        if( ref->getSource() == master)
            continue;

        if( ref->getSource() == bot)
            return 2 * M_PI / (group->GetMembersCount() -1) * index;

        index++;
    }
    return 0;
}

bool MovementAction::IsMovingAllowed(Unit* target)
{
    if (!target)
        return false;

    if (bot->GetMapId() != target->GetMapId())
        return false;

    float distance = sServerFacade.GetDistance2d(bot, target);
    if (!bot->InBattleGround() && distance > sPlayerbotAIConfig.reactDistance)
        return false;

    return IsMovingAllowed();
}

bool MovementAction::IsMovingAllowed(uint32 mapId, float x, float y, float z)
{
    float distance = sqrt(bot->GetDistance(x, y, z));
    if (!bot->InBattleGround() && distance > sPlayerbotAIConfig.reactDistance)
        return false;

    return IsMovingAllowed();
}

bool MovementAction::IsMovingAllowed()
{
    // do not allow if not vehicle driver
    if (ai->IsInVehicle() && !ai->IsInVehicle(true))
        return false;

    // test
    return ai->CanMove();

    if (sServerFacade.IsFrozen(bot) || bot->IsPolymorphed() ||
			(sServerFacade.UnitIsDead(bot) && !bot->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST)) ||
            bot->IsBeingTeleported() ||
            sServerFacade.IsInRoots(bot) ||
            bot->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION) ||
            bot->HasAuraType(SPELL_AURA_MOD_CONFUSE) || sServerFacade.IsCharmed(bot) ||
            bot->HasAuraType(SPELL_AURA_MOD_STUN) || bot->IsTaxiFlying() ||
            bot->hasUnitState(UNIT_STAT_CAN_NOT_REACT_OR_LOST_CONTROL))// ||
            //bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FALLING))
        return false;

    MotionMaster &mm = *bot->GetMotionMaster();
#ifdef CMANGOS
    return mm.GetCurrentMovementGeneratorType() != TAXI_MOTION_TYPE;
#endif
#ifdef MANGOS
    return mm.GetCurrentMovementGeneratorType() != FLIGHT_MOTION_TYPE;
#endif
}

bool MovementAction::Follow(Unit* target, float distance)
{
    if (!distance)
        distance = ai->GetRange("follow");
    return Follow(target, distance, GetFollowAngle());
}

void MovementAction::UpdateMovementState()
{
    if (bot->IsInWater() || sServerFacade.IsUnderwater(bot))
    {
		bot->m_movementInfo.AddMovementFlag(MOVEFLAG_SWIMMING);
        bot->UpdateSpeed(MOVE_SWIM, true);
    }
    else
    {
		bot->m_movementInfo.RemoveMovementFlag(MOVEFLAG_SWIMMING);
        bot->UpdateSpeed(MOVE_SWIM, true);
    }

#ifndef MANGOSBOT_ZERO
    if (bot->IsFlying())
        bot->UpdateSpeed(MOVE_FLIGHT, true);
#endif

    // Temporary speed increase in group
    //if (ai->HasRealPlayerMaster())
    //    bot->UpdateSpeed(MOVE_RUN, true, 1.1f);
    
    // check if target is not reachable (from Vmangos)
    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == CHASE_MOTION_TYPE &&
        !bot->GetMotionMaster()->GetCurrent()->IsReachable() && !bot->InBattleGround())
    {
        if (Unit* pTarget = bot->GetMotionMaster()->GetCurrent()->GetCurrentTarget())
        {
            if (!bot->CanReachWithMeleeAttack(pTarget) && pTarget->IsCreature())
            {
                float angle = bot->GetAngle(pTarget);
                float distance = 5.0f;
                float x = bot->GetPositionX() + cos(angle) * distance,
                    y = bot->GetPositionY() + sin(angle) * distance,
                    z = bot->GetPositionZ();

                z += CONTACT_DISTANCE;
                bot->UpdateAllowedPositionZ(x, y, z);

                ai->StopMoving();
                bot->NearTeleportTo(x, y, z, bot->GetOrientation());
                //bot->GetMotionMaster()->MovePoint(bot->GetMapId(), x, y, z, FORCED_MOVEMENT_RUN, false);
                return;
                /*if (pTarget->IsCreature() && !bot->IsMoving() && bot->IsWithinDist(pTarget, 10.0f))
                {
                    // Cheating to prevent getting stuck because of bad mmaps.
                    bot->StopMoving();
                    bot->GetMotionMaster()->Clear();
                    bot->GetMotionMaster()->MovePoint(bot->GetMapId(), pTarget->GetPosition(), FORCED_MOVEMENT_RUN, false);
                    return;
                }*/
            }
        }
    }

    if ((bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE ||
        bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE ) &&
        !bot->GetMotionMaster()->GetCurrent()->IsReachable() && !bot->InBattleGround())
    {
        if (Unit* pTarget = bot->GetMotionMaster()->GetCurrent()->GetCurrentTarget())
        {
            if (pTarget != ai->GetGroupMaster())
                return;

            if (!bot->CanReachWithMeleeAttack(pTarget))
            {
                if (bot->IsStopped() && bot->IsWithinDist(pTarget, 10.0f))
                {
                    // Cheating to prevent getting stuck because of bad mmaps.
                    float angle = bot->GetAngle(pTarget);
                    float distance = 5.0f;
                    float x = bot->GetPositionX() + cos(angle) * distance,
                        y = bot->GetPositionY() + sin(angle) * distance,
                        z = bot->GetPositionZ();

                    z += CONTACT_DISTANCE;
                    bot->UpdateAllowedPositionZ(x, y, z);

                    ai->StopMoving();
                    bot->NearTeleportTo(x, y, z, bot->GetOrientation());
                    //bot->GetMotionMaster()->MovePoint(bot->GetMapId(), x, y, z, FORCED_MOVEMENT_RUN, false);
                    return;
                }
            }
        }
    }
}

bool MovementAction::Follow(Unit* target, float distance, float angle)
{
    MotionMaster &mm = *bot->GetMotionMaster();

    distance = distance <= target->GetObjectBoundingRadius() ? 0 : distance - target->GetObjectBoundingRadius();

    UpdateMovementState();

    if (!target)
        return false;

    //Move to target corpse if alive.
    if (!target->IsAlive() && bot->IsAlive() && target->GetObjectGuid().IsPlayer())
    {
        Player* pTarget = (Player*)target;

        Corpse* corpse = pTarget->GetCorpse();

        if (corpse)
        {
            WorldPosition botPos(bot);
            WorldPosition cPos(corpse);

            if(botPos.fDist(cPos) > sPlayerbotAIConfig.spellDistance)
                return MoveTo(cPos.getMapId(),cPos.getX(),cPos.getY(), cPos.getZ());
            return false;
        }
    }

    if (sServerFacade.IsDistanceGreaterOrEqualThan(sServerFacade.GetDistance2d(bot, target), sPlayerbotAIConfig.sightDistance) || (target->IsFlying() && !bot->IsFreeFlying()) || target->IsTaxiFlying())
    {
        if (target->GetObjectGuid().IsPlayer())
        {
            Player* pTarget = (Player*)target;

            if (pTarget->GetPlayerbotAI()) //Try to move to where the bot is going if it is closer and in the same direction.
            {
                WorldPosition botPos(bot);
                WorldPosition tarPos(target);
                WorldPosition longMove = pTarget->GetPlayerbotAI()->GetAiObjectContext()->GetValue<WorldPosition>("last long move")->Get();

                if (longMove)
                {
                    float lDist = botPos.fDist(longMove);
                    float tDist = botPos.fDist(tarPos);
                    float ang = botPos.getAngleBetween(tarPos, longMove);
                    if ((lDist * 1.5 < tDist && ang < M_PI_F / 2) || target->IsTaxiFlying())
                    {
                        return MoveTo(longMove.getMapId(), longMove.getX(), longMove.getY(), longMove.getZ());
                    }
                }
            }

            if (pTarget->IsTaxiFlying()) //Move to where the player is flying to.
            {
                const Taxi::Map tMap = pTarget->GetTaxiPathSpline();
                if (!tMap.empty())
                {
                    auto tEnd = tMap.back();

                    if (tEnd)
                        return MoveTo(tEnd->mapid, tEnd->x, tEnd->y, tEnd->z);
                }
            }
        }

        if (!target->IsTaxiFlying())
           return MoveTo(target, ai->GetRange("follow"));
    }

    bot->HandleEmoteState(0);
    if (!bot->IsStandState())
        bot->SetStandState(UNIT_STAND_STATE_STAND);

    if (bot->IsNonMeleeSpellCasted(true))
    {
        bot->CastStop();
        ai->InterruptSpell();
    }

    AI_VALUE(LastMovement&, "last movement").Set(target);
    ClearIdleState();

#ifndef MANGOSBOT_ZERO
    if (bot->IsFreeFlying())
    {
        if (!bot->IsFlying() && target->IsFlying())
        {
            //Take off
            WorldPacket data(SMSG_SPLINE_MOVE_SET_FLYING, 9);
            data << bot->GetPackGUID();
            bot->SendMessageToSet(data, true);

            if (!bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FLYING))
                bot->m_movementInfo.AddMovementFlag(MOVEFLAG_FLYING);
#ifdef MANGOSBOT_ONE
            if (!bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FLYING2))
                bot->m_movementInfo.AddMovementFlag(MOVEFLAG_FLYING2);
#endif
            if (!bot->m_movementInfo.HasMovementFlag(MOVEFLAG_LEVITATING))
                bot->m_movementInfo.AddMovementFlag(MOVEFLAG_LEVITATING);
        }

        if (bot->IsFlying() && !target->IsFlying())
        {
            //Land
            bool needLand = false;

            if (const TerrainInfo* terrain = bot->GetTerrain())
            {
                float height = terrain->GetHeightStatic(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
                float ground = terrain->GetWaterOrGroundLevel(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), height);
                if (bot->GetPositionZ() < ground + 5.0f)
                    needLand = true;
            }
            if (needLand)
            {
                WorldPacket data(SMSG_SPLINE_MOVE_UNSET_FLYING, 9);
                data << bot->GetPackGUID();
                bot->SendMessageToSet(data, true);

                if (bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FLYING))
                    bot->m_movementInfo.RemoveMovementFlag(MOVEFLAG_FLYING);
#ifdef MANGOSBOT_ONE
                if (bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FLYING2))
                    bot->m_movementInfo.RemoveMovementFlag(MOVEFLAG_FLYING2);
#endif
                if (bot->m_movementInfo.HasMovementFlag(MOVEFLAG_LEVITATING))
                    bot->m_movementInfo.RemoveMovementFlag(MOVEFLAG_LEVITATING);

                if(!bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FALLING))
                    bot->m_movementInfo.RemoveMovementFlag(MOVEFLAG_FALLING);
            }
        }
    }
#endif

    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
    {
        Unit* currentTarget = sServerFacade.GetChaseTarget(bot);
        if (currentTarget && currentTarget->GetObjectGuid() == target->GetObjectGuid() && sServerFacade.GetChaseAngle(bot) == angle && sServerFacade.GetChaseOffset(bot) == distance)
            return false;
    }

    mm.MoveFollow(target, distance, angle, true, sPlayerbotAIConfig.boostFollow);
    return true;
}

Vector3 CalculatePerpendicularPoint(const Vector3& A, const Vector3& B, float offset, bool left = true)
{
    const Vector3 direction = (B - A).directionOrZero();
    Vector3 perpendicularDirection(-direction.y, direction.x, direction.z);

    if (!left)
    {
        perpendicularDirection.x *= -1.0f;
        perpendicularDirection.y *= -1.0f;
    }

    return B + (perpendicularDirection * offset);
}

bool MovementAction::ChaseTo(WorldObject* obj, float distance, float angle)
{
    if (!IsMovingAllowed())
    {
        return false;
    }

    if (!obj || obj == bot || bot->GetMapId() != obj->GetMapId())
        return false;

#ifdef MANGOSBOT_TWO
    TransportInfo* transportInfo = bot->GetTransportInfo();
    if (transportInfo && transportInfo->IsOnVehicle())
    {
        Unit* vehicle = (Unit*)transportInfo->GetTransport();
        VehicleSeatEntry const* seat = vehicle->GetVehicleInfo()->GetSeatEntry(transportInfo->GetTransportSeat());
        if (!seat || !seat->HasFlag(SEAT_FLAG_CAN_CONTROL))
            return false;

        //vehicle->GetMotionMaster()->Clear();
        return MoveNear(obj, 30.0f);
        //vehicle->GetMotionMaster()->MoveChase((Unit*)obj, 30.0f, angle);
        //return true;
    }
#endif

    if (ai->HasStrategy("behind", BotState::BOT_STATE_COMBAT))
        angle = GetFollowAngle() / 3 + obj->GetOrientation() + M_PI;

    UpdateMovementState();

    bot->HandleEmoteState(0);
    if (!bot->IsStandState())
        bot->SetStandState(UNIT_STAND_STATE_STAND);

#ifndef MANGOSBOT_ZERO
    if (bot->InArena())
        return MoveNear(obj, std::max(ATTACK_DISTANCE, distance));
#endif

    // Calculate the chase position
    const WorldPosition botPosition(bot);
    const WorldPosition targetPosition(obj);
    const Vector3 botPoint = botPosition.getVector3();
    const Vector3 targetPoint = targetPosition.getVector3();

    const float distanceToTarget = botPosition.distance(targetPosition);
    const Vector3 directionToTarget = (targetPoint - botPoint).directionOrZero();
    const Vector3 endPoint = botPoint + (directionToTarget * std::min(distance, distanceToTarget));
    WorldPosition endPosition(obj->GetMapId(), endPoint.x, endPoint.y, endPoint.z);

    // Check if the end position is inside a hazard
    HazardPosition hazardPosition;
    if (IsHazardNearPosition(endPosition, &hazardPosition))
    {
        // Try to generate a nearby position outside the hazard
        const Vector3 hazardPoint = hazardPosition.first.getVector3();
        const float hazardRangeOffset = hazardPosition.second * 1.5f;

        // Generate point translated to the left
        Vector3 possibleEndPoint = CalculatePerpendicularPoint(endPoint, hazardPoint, hazardRangeOffset, true);

        // Check if point is valid
        WorldPosition possibleEndPosition (bot->GetMapId(), possibleEndPoint.x, possibleEndPoint.y, possibleEndPoint.z);
        if (IsValidPosition(possibleEndPosition, botPosition))
        {
            endPosition.coord_x = possibleEndPoint.x;
            endPosition.coord_y = possibleEndPoint.y;
            endPosition.coord_z = possibleEndPoint.z;
        }
        else
        {
            // Generate point translated to the right
            possibleEndPoint = CalculatePerpendicularPoint(endPoint, hazardPoint, hazardRangeOffset, false);
        
            endPosition.coord_x = possibleEndPoint.x;
            endPosition.coord_y = possibleEndPoint.y;
            endPosition.coord_z = possibleEndPoint.z;
        }
    }

    // Prevent moving if requested to move into a hazard
    if (IsValidPosition(endPosition, botPosition))
    {
        MotionMaster& mm = *bot->GetMotionMaster();
        mm.Clear(false, true);

        Movement::PointsArray path;
        if (GeneratePathAvoidingHazards(endPosition, true, path))
        {
            mm.MovePath(path, FORCED_MOVEMENT_RUN, false, false);
            WaitForReach(path);
        }
        else
        {
            if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == CHASE_MOTION_TYPE)
            {
                ChaseMovementGenerator* chase = (ChaseMovementGenerator*)bot->GetMotionMaster()->GetCurrent();

                if (chase->GetCurrentTarget() == obj)
                    return true;
            }

            mm.MoveChase((Unit*)obj, distance, angle);
            float dist = sServerFacade.GetDistance2d(bot, obj);
            float distDiff = dist > distance ? dist - distance : 0.f;
            WaitForReach(distDiff);
        }

        return true;
    }
   
    SetDuration(sPlayerbotAIConfig.reactDelay);
    return false;
}

float MovementAction::MoveDelay(float distance)
{
    return distance / bot->GetSpeed(MOVE_RUN);
}

void MovementAction::WaitForReach(float distance)
{
    float duration = 1000.0f * MoveDelay(distance) + sPlayerbotAIConfig.reactDelay;
    if (duration > sPlayerbotAIConfig.maxWaitForMove)
        duration = sPlayerbotAIConfig.maxWaitForMove;

    /*Unit* target = *ai->GetAiObjectContext()->GetValue<Unit*>("current target");
    Unit* player = *ai->GetAiObjectContext()->GetValue<Unit*>("enemy player target");
    if ((player || target) && duration > sPlayerbotAIConfig.globalCoolDown)
        duration = sPlayerbotAIConfig.globalCoolDown;*/

    if (duration < 0.0f)
        duration = 0.0f;

    SetDuration(duration);
}

void MovementAction::WaitForReach(const Movement::PointsArray& path)
{
    float distance = 0.0f;
    if(!path.empty())
    {
        const Vector3* previousPoint = &path[0]; 
        for (auto it = path.begin() + 1; it != path.end(); ++it)
        {
            const Vector3& pathPoint = (*it);
            distance += (*previousPoint - pathPoint).length();
            previousPoint = &pathPoint;
        }
    }

    WaitForReach(distance);
}

bool MovementAction::Flee(Unit *target)
{
    Player* master = GetMaster();
    if (!target)
        target = master;

    if (!target)
        return false;

    if (!sPlayerbotAIConfig.fleeingEnabled)
        return false;

    if (!IsMovingAllowed())
    {
        ai->TellError("I am stuck while fleeing");
        return false;
    }

    time_t lastFlee = AI_VALUE(LastMovement&, "last movement").lastFlee;
    time_t now = time(0);
    uint32 fleeDelay = urand(2, sPlayerbotAIConfig.returnDelay / 1000);

    if (lastFlee && bot->IsMoving())
    {
        if ((now - lastFlee) <= fleeDelay)
        {
            return true;
        }
    }
    
    const bool isHealer = ai->IsHeal(bot);
    const bool isTank = ai->IsTank(bot);
    const bool isDps = !isHealer && !isTank;
    const bool isRanged = ai->IsRanged(bot);
    const bool needHealer = !isHealer && AI_VALUE2(uint8, "health", "self target") < 50;

    HostileReference *ref = sServerFacade.GetThreatManager(target).getCurrentVictim();
    const bool isTarget = ref && ref->getTarget() == bot;

    Unit* fleeTarget = nullptr;
    Group* group = bot->GetGroup();
    if (group)
    {
        Unit* spareTarget = nullptr;
        vector<Unit*> possibleTargets;
        const float minFleeDistance = 5.0f;
        const float maxFleeDistance = isTarget ? 40.0f : ai->GetRange("spell") * 1.5;
        const float minRangedTargetDistance = ai->GetRange("spell") / 2 + ai->GetRange("follow");

        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* groupMember = gref->getSource();

            // Ignore group member if is not alive or on a different zone
            if (!groupMember || groupMember->IsBeingTeleported() || groupMember == bot || groupMember == master || !sServerFacade.IsAlive(groupMember) || bot->GetMapId() != groupMember->GetMapId())
                continue;

            // Don't flee to group member if too close or too far
            float const distanceToGroupMember = sServerFacade.GetDistance2d(bot, groupMember);
            if (distanceToGroupMember < minFleeDistance || distanceToGroupMember > maxFleeDistance)
                continue;

            if (PlayerbotAI* groupMemberBotAi = groupMember->GetPlayerbotAI())
            {
                // Ignore if the group member is affected by an aoe spell
                if (groupMemberBotAi->GetAiObjectContext()->GetValue<bool>("has area debuff", "self target")->Get())
                    continue;
            }

            // If the bot is currently being targeted
            if(isTarget)
            {
                // Try to flee to tank
                if (ai->IsTank(groupMember))
                {
                    float distanceToTank = sServerFacade.GetDistance2d(bot, groupMember);
                    float distanceToTarget = sServerFacade.GetDistance2d(bot, target);
                    if (distanceToTank > minFleeDistance && distanceToTank < maxFleeDistance)
                    {
                        possibleTargets.push_back(groupMember);
                    }
                }
            }
            else
            {
                // Try to flee to healers (group healers together or approach a healer if needed)
                if ((isHealer && ai->IsHeal(groupMember)) || needHealer)
                {
                    const float distanceToTarget = sServerFacade.GetDistance2d(groupMember, target);
                    if (distanceToTarget > minRangedTargetDistance && (needHealer || groupMember->IsWithinLOSInMap(target, true)))
                    {
                        possibleTargets.push_back(groupMember);
                    }
                }
                // Try to flee to ranged (group ranged together)
                else if (isRanged && ai->IsRanged(groupMember))
                {
                    const float distanceToTarget = sServerFacade.GetDistance2d(groupMember, target);
                    if (distanceToTarget > minRangedTargetDistance && groupMember->IsWithinLOSInMap(target, true))
                    {
                        possibleTargets.push_back(groupMember);
                    }
                }
            }
        }

        if (!possibleTargets.empty())
        {
            fleeTarget = possibleTargets[urand(0, possibleTargets.size() - 1)];
        }
        else
        {
            // If nothing was found, let's try the master
            if (master && sServerFacade.IsAlive(master) && master->IsWithinLOSInMap(target, true))
            {
                // Don't flee to group member if too close or too far
                float const distanceToMaster = sServerFacade.GetDistance2d(bot, master);
                if (distanceToMaster > minFleeDistance && distanceToMaster < maxFleeDistance)
                {
                    if(isRanged)
                    {
                        const float distanceToTarget = sServerFacade.GetDistance2d(master, target);
                        if (distanceToTarget > minRangedTargetDistance)
                        {
                            fleeTarget = master;
                        }
                    }
                    else
                    {
                        fleeTarget = master;
                    }
                }
            }
        }
    }

    bool succeeded = false;
    if (fleeTarget)
    {
        succeeded = MoveNear(fleeTarget);
    }

    if (!ai->HasRealPlayerMaster() && !ai->IsRealPlayer(target))
    {
        bool fullDistance = false;
        if (target->IsPlayer())
            fullDistance = true;
        if (WorldPosition(bot).isOverworld())
            fullDistance = true;

        float distance = fullDistance ? (ai->GetRange("flee") * 2) : ai->GetRange("flee");

        MotionMaster* mm = bot->GetMotionMaster();

        if (mm->GetCurrentMovementGeneratorType() == CHASE_MOTION_TYPE)
        {
            ChaseMovementGenerator* chase = (ChaseMovementGenerator*)mm->GetCurrent();

            if (chase->GetCurrentTarget() == target && sServerFacade.GetChaseOffset(bot) == distance)
                return true;
        }

        ai->StopMoving();
        mm->MoveChase(target, distance, WorldPosition(bot).getAngleTo(target), true, false, true, false);
        return true;
    }

    // Generate a position to flee
    if(!succeeded)
    {
        if (lastFlee && bot->GetGroup())
        {
            if (!lastFlee)
            {
                AI_VALUE(LastMovement&, "last movement").lastFlee = now;
            }
            else
            {
                if ((now - lastFlee) > fleeDelay)
                {
                    AI_VALUE(LastMovement&, "last movement").lastFlee = 0;
                }
                else
                {
                    succeeded = false;
                }
            }
        }
        bool fullDistance = false;
        if (target->IsPlayer())
            fullDistance = true;
        if (WorldPosition(bot).isOverworld())
            fullDistance = true;

        FleeManager manager(bot, fullDistance ? (ai->GetRange("flee") * 2) : ai->GetRange("flee"), bot->GetAngle(target) + M_PI);
        if (!manager.isUseful())
        {
            return false;
        }

        if (!urand(0, 50) && ai->HasStrategy("emote", BotState::BOT_STATE_NON_COMBAT))
        {
            vector<uint32> sounds;
            sounds.push_back(304); // guard
            sounds.push_back(306); // flee
            ai->PlayEmote(sounds[urand(0, sounds.size() - 1)]);
        }

        float rx, ry, rz;
        if (!manager.CalculateDestination(&rx, &ry, &rz))
        {
            ai->TellError("Nowhere to flee");
            return false;
        }

        if(MoveTo(target->GetMapId(), rx, ry, rz))
        {
            AI_VALUE(LastMovement&, "last movement").lastFlee = time(0);
            succeeded = true;
        }
    }

    return succeeded;
}

void MovementAction::ClearIdleState()
{
    context->GetValue<time_t>("stay time")->Set(0);
    context->GetValue<ai::PositionMap&>("position")->Get()["random"].Reset();
}

bool MovementAction::IsValidPosition(const WorldPosition& position, const WorldPosition& visibleFromPosition)
{
    const WorldPosition botPosition(bot);
    return botPosition.canPathTo(position, bot) &&
           MaNGOS::IsValidMapCoord(position.getX(), position.getY(), position.getZ(), 0.0f) &&
           position.IsInLineOfSight(visibleFromPosition, bot->GetCollisionHeight()) &&
           !IsHazardNearPosition(position);
}

bool MovementAction::IsHazardNearPosition(const WorldPosition& position, HazardPosition* outHazard)
{
    AiObjectContext* context = bot->GetPlayerbotAI()->GetAiObjectContext();
    list<HazardPosition>& hazards = AI_VALUE(list<HazardPosition>, "hazards");
    if (!hazards.empty())
    {
        for (const HazardPosition& hazard : hazards)
        {
            const WorldPosition& hazardPosition = hazard.first;
            const float hazardRange = hazard.second;
            const float distance = position.distance(hazardPosition);
            if (distance <= hazardRange)
            {
                if (outHazard)
                {
                    *outHazard = hazard;
                }

                return true;
            }
        }
    }

    return false;
}

bool MovementAction::GeneratePathAvoidingHazards(const WorldPosition& endPosition, bool generatePath, Movement::PointsArray& outPath)
{
    if (generatePath)
    {
        list<HazardPosition>& hazards = AI_VALUE(list<HazardPosition>, "hazards");
        if (!hazards.empty())
        {
            PathFinder path(bot);
            path.calculate(endPosition.getX(), endPosition.getY(), endPosition.getZ(), false);
            Movement::PointsArray& pathPoints = path.getPath();
            Movement::PointsArray collidingHazards;

            bool pathModified = false;
            if(pathPoints.size() >= 2)
            {
                if (pathPoints.size() == 2)
                {
                    pathPoints.push_back(pathPoints.back());
                }

                // Start the iteration on the second point (the first and last points can't be modified)
                bool firstPoint = true;
                uint8 pointsInserted = 0;
                const uint8 maxPointsInserted = 20;
                WorldPosition previousPosition(bot->GetMapId(), pathPoints.begin()->x, pathPoints.begin()->y, pathPoints.begin()->z);
                for (uint32 i = 1; i < pathPoints.size() - 1; i++)
                {
                    bool pointInserted = false;
                    Vector3& pathPoint = pathPoints[i];
                    for (const HazardPosition& hazard : hazards)
                    {
                        const float hazardRange = hazard.second;
                        const float hazardRangeOffset = hazardRange * 1.5f;
                        const Vector3& hazardPosition = hazard.first.getVector3();

                        // Check if the path point is inside a hazard
                        {
                            const float distanceToHazard = (pathPoint - hazardPosition).length();
                            if (distanceToHazard <= hazardRange)
                            {
                                collidingHazards.push_back(hazardPosition);

                                // Move the point out of the hazard range in perpendicular from previous point
                                // Generate point translated to the left
                                Vector3 possiblePathPoint = CalculatePerpendicularPoint(previousPosition.getVector3(), hazardPosition, hazardRangeOffset, true);

                                // Check if point is valid
                                WorldPosition possiblePathPosition(bot->GetMapId(), possiblePathPoint.x, possiblePathPoint.y, possiblePathPoint.z);
                                if (IsValidPosition(possiblePathPosition, previousPosition))
                                {
                                    pathModified = true;
                                    pathPoint = possiblePathPoint;
                                }
                                else
                                {
                                    // Generate point translated to the right
                                    possiblePathPoint = CalculatePerpendicularPoint(previousPosition.getVector3(), hazardPosition, hazardRangeOffset, false);

                                    // Check if point is valid
                                    WorldPosition possiblePathPosition(bot->GetMapId(), possiblePathPoint.x, possiblePathPoint.y, possiblePathPoint.z);
                                    if (IsValidPosition(possiblePathPosition, previousPosition))
                                    {
                                        pathModified = true;
                                        pathPoint = possiblePathPoint;
                                    }
                                }
                            }
                        }

                        // Check if the line between the previous point and the current point goes through a hazard
                        // Don't check for the line between the first point and second
                        if (!firstPoint && (pointsInserted < maxPointsInserted))
                        {
                            Vector3 directionFromPreviousPoint = (pathPoint - previousPosition.getVector3());
                            const float distanceToPreviousPoint = std::max(directionFromPreviousPoint.length(), 0.0001f);
                            directionFromPreviousPoint = directionFromPreviousPoint / distanceToPreviousPoint;
                            Vector3 inBetweenPathPoint = previousPosition.getVector3() + (directionFromPreviousPoint * distanceToPreviousPoint * 0.5f);

                            // Check if the point between path points is inside a hazard
                            Vector3 directionFromHazard = (inBetweenPathPoint - hazardPosition);
                            const float distanceToHazard = std::max(directionFromHazard.length(), 0.0001f);
                            if (distanceToHazard <= hazardRange)
                            {
                                collidingHazards.push_back(hazardPosition);

                                // If so generate a new path point to go around it
                                inBetweenPathPoint = hazardPosition + ((directionFromHazard / distanceToHazard) * hazardRangeOffset);

                                // Check if the point is valid
                                WorldPosition inBetweenPathPosition(bot->GetMapId(), inBetweenPathPoint.x, inBetweenPathPoint.y, inBetweenPathPoint.z);
                                if (IsValidPosition(inBetweenPathPosition, previousPosition))
                                {
                                    // Insert the new point to the path (before current point)
                                    pathModified = true;
                                    pointInserted = true;
                                    pathPoints.insert(pathPoints.begin() + i, inBetweenPathPoint);
                                    pointsInserted++;
                                    continue;
                                }
                            }
                        }
                    }

                    if(pointInserted)
                    {
                        // Go back one step to validate the inserted point and move to next loop
                        i--;
                    }
                    else
                    {
                        firstPoint = false;
                        previousPosition.coord_x = pathPoint.x;
                        previousPosition.coord_y = pathPoint.y;
                        previousPosition.coord_z = pathPoint.z;
                    }
                }

                if (pathModified && !pathPoints.empty())
                {
                    outPath = pathPoints;

                    if (ai->HasStrategy("debug move", BotState::BOT_STATE_COMBAT))
                    {
                        for (const Vector3& pathPoint : pathPoints)
                        {
                            bot->SummonCreature(1, pathPoint.x, pathPoint.y, pathPoint.z, 0.0f, TEMPSPAWN_TIMED_DESPAWN, 5000.0f);
                        }

                        for (const Vector3& hazards : collidingHazards)
                        {
                            bot->SummonCreature(15631, hazards.x, hazards.y, hazards.z, 0.0f, TEMPSPAWN_TIMED_DESPAWN, 5000.0f);
                        }
                    }

                    return true;
                }
            }
        }
    }

    return false;
}

bool FleeAction::Execute(Event& event)
{
    return Flee(AI_VALUE(Unit*, "current target"));
}

bool FleeWithPetAction::Execute(Event& event)
{
    Pet* pet = bot->GetPet();
    if (pet)
    {
#ifdef MANGOS
        CreatureAI*
#endif
#ifdef CMANGOS
            UnitAI*
#endif
            creatureAI = ((Creature*)pet)->AI();
        if (creatureAI)
        {
#ifdef CMANGOS
            creatureAI->SetReactState(REACT_PASSIVE);
#endif
#ifdef MANGOS
            pet->GetCharmInfo()->SetReactState(REACT_PASSIVE);
            pet->GetCharmInfo()->SetCommandState(COMMAND_FOLLOW);
#endif
            pet->AttackStop();
        }
    }
    return Flee(AI_VALUE(Unit*, "current target"));
}

bool RunAwayAction::Execute(Event& event)
{
    return Flee(AI_VALUE(Unit*, "master target"));
}

bool MoveToLootAction::Execute(Event& event)
{
    LootObject loot = AI_VALUE(LootObject, "loot target");
    if (!loot.IsLootPossible(bot))
        return false;

    WorldObject *wo = loot.GetWorldObject(bot);
    return MoveNear(wo, sPlayerbotAIConfig.contactDistance);
}

bool MoveOutOfEnemyContactAction::Execute(Event& event)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    return MoveTo(target, sPlayerbotAIConfig.contactDistance);
}

bool MoveOutOfEnemyContactAction::isUseful()
{
    return AI_VALUE2(bool, "inside target", "current target");
}

bool SetFacingTargetAction::Execute(Event& event)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    if (bot->IsTaxiFlying())
        return true;

    sServerFacade.SetFacingTo(bot, target);
    //SetDuration(sPlayerbotAIConfig.globalCoolDown);
    return true;
}

bool SetFacingTargetAction::isUseful()
{
    return !AI_VALUE2(bool, "facing", "current target");
}

bool SetFacingTargetAction::isPossible()
{
    if (sServerFacade.IsFrozen(bot) || bot->IsPolymorphed() ||
        (sServerFacade.UnitIsDead(bot) && !bot->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST)) ||
        bot->IsBeingTeleported() ||
        bot->HasAuraType(SPELL_AURA_MOD_CONFUSE) || sServerFacade.IsCharmed(bot) ||
        bot->HasAuraType(SPELL_AURA_MOD_STUN) || bot->IsTaxiFlying() ||
        bot->hasUnitState(UNIT_STAT_CAN_NOT_REACT_OR_LOST_CONTROL))
        return false;

    return true;
}

bool SetBehindTargetAction::Execute(Event& event)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    float angle = GetFollowAngle() / 3 + target->GetOrientation() + M_PI / 2.0f;

    float distance = bot->GetCombinedCombatReach(target, true) * 0.8f;
    float x = target->GetPositionX() + cos(target->GetOrientation()) * -1.0f * distance,
        y = target->GetPositionY() + sin(target->GetOrientation()) * -1.0f * distance,
        z = target->GetPositionZ();
    bot->UpdateGroundPositionZ(x, y, z);

    // prevent going into terrain
    float ox, oy, oz;
    target->GetPosition(ox, oy, oz);
#ifdef MANGOSBOT_TWO
    target->GetMap()->GetHitPosition(ox, oy, oz + bot->GetCollisionHeight(), x, y, z, bot->GetPhaseMask(), -0.5f);
#else
    target->GetMap()->GetHitPosition(ox, oy, oz + bot->GetCollisionHeight(), x, y, z, -0.5f);
#endif

    const bool isLos = target->IsWithinLOS(x, y, z + bot->GetCollisionHeight(), true);
    bool moved = MoveTo(bot->GetMapId(), x, y, z);
    if (!moved && !isLos)
    {
        distance = sPlayerbotAIConfig.contactDistance;
        x = target->GetPositionX() + cos(angle) * distance;
        y = target->GetPositionY() + sin(angle) * distance;
        z = target->GetPositionZ();
        bot->UpdateGroundPositionZ(x, y, z);
        moved = MoveTo(bot->GetMapId(), x, y, z);
    }

    return moved;
}

bool SetBehindTargetAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    // Do not move if stay strategy is set
    if(ai->HasStrategy("stay", ai->GetState()))
        return false;

    return !bot->IsFacingTargetsBack(target);
}

bool SetBehindTargetAction::isPossible()
{
    if(MovementAction::isPossible())
    {
        // Check if the target is targeting the bot
        Unit* target = AI_VALUE(Unit*, "current target");
        if (target)
        {
            // If the target is a player
            Player* playerTarget = dynamic_cast<Player*>(target);
            if(playerTarget)
            {
                return bot->GetObjectGuid() != playerTarget->GetSelectionGuid();
            }
            // If the target is a NPC
            else 
            {
                return !(target->GetVictim() && (target->GetVictim()->GetObjectGuid() == bot->GetObjectGuid()));
            }
        }
    }

    return false;
}

bool MoveOutOfCollisionAction::Execute(Event& event)
{
    float angle = M_PI * 2000 / (float)urand(1, 1000);
    float distance = ai->GetRange("follow");
    return MoveTo(bot->GetMapId(), bot->GetPositionX() + cos(angle) * distance, bot->GetPositionY() + sin(angle) * distance, bot->GetPositionZ());
}

bool MoveOutOfCollisionAction::isUseful()
{
#ifdef MANGOSBOT_TWO
    // do not avoid collision on vehicle
    if (ai->IsInVehicle())
        return false;
#endif

    return AI_VALUE2(bool, "collision", "self target") && ai->GetAiObjectContext()->GetValue<list<ObjectGuid> >("nearest friendly players")->Get().size() < 15 &&
        ai->GetAiObjectContext()->GetValue<list<ObjectGuid> >("nearest non bot players")->Get().size() > 0;
}

bool MoveRandomAction::Execute(Event& event)
{
    //uint32 randnum = bot->GetGUIDLow();                            //Semi-random but fixed number for each bot.
    //uint32 cycle = floor(WorldTimer::getMSTime() / (1000*60));     //Semi-random number adds 1 each minute.

    //randnum = ((randnum + cycle) % 1000) + 1;

    uint32 randnum = urand(1, 2000);

    float angle = M_PI  * (float)randnum / 1000; //urand(1, 1000);
    float distance = urand(20,200);

    return MoveTo(bot->GetMapId(), bot->GetPositionX() + cos(angle) * distance, bot->GetPositionY() + sin(angle) * distance, bot->GetPositionZ());
}

bool MoveRandomAction::isUseful()
{    
    return !ai->HasRealPlayerMaster() && ai->GetAiObjectContext()->GetValue<list<ObjectGuid> >("nearest friendly players")->Get().size() > urand(25, 100);
}

bool MoveToAction::Execute(Event& event)
{
    list<GuidPosition> guidList = AI_VALUE(list<GuidPosition>, getQualifier());

    if (guidList.empty())
        return false;

    GuidPosition guid = guidList.front();

    return MoveTo(guid.getMapId(), guid.getX(), guid.getY(), guid.getZ());
}