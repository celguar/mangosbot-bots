#include "botpch.h"
#include "../../playerbot.h"
#include "CheckMountStateAction.h"
#include "../values/PositionValue.h"
#include "../../ServerFacade.h"
#include "BattleGroundWS.h"
#include "../../TravelMgr.h"

using namespace ai;

uint64 extractGuid(WorldPacket& packet);

bool CheckMountStateAction::Execute(Event& event)
{
    Player* groupMaster = ai->GetGroupMaster();

    bool hasAttackers = AI_VALUE(bool, "has attackers");
    bool hasEnemy = AI_VALUE(bool, "has enemy player targets") || AI_VALUE(Unit*, "dps target");
    TravelTarget* travelTarget = AI_VALUE(TravelTarget*, "travel target");

    bool canFly = CanFly();

    bool farFromMaster = false;

    if(groupMaster)
        farFromMaster = bot->GetMapId() != groupMaster->GetMapId() || bot->GetDistance(groupMaster) > sPlayerbotAIConfig.sightDistance;

    bool canAttackTarget = false;
    bool shouldChaseTarget = false;
    bool farFromTarget = false;

    if (hasEnemy)
    {
        float distToTarget = AI_VALUE2(float, "distance", "current target");
        canAttackTarget = sServerFacade.IsDistanceLessThan(distToTarget, GetAttackDistance());
        shouldChaseTarget = sServerFacade.IsDistanceGreaterThan(distToTarget, 45.0f) && AI_VALUE2(bool, "moving", "current target");
        farFromTarget = sServerFacade.IsDistanceGreaterThan(distToTarget, 40.0f);
    }

    //Mount up in battle grounds
    if (bot->InBattleGround())
    {
        if (!canAttackTarget)
        {
            if (!hasAttackers || (farFromTarget && !bot->IsInCombat()))
            {
                if (CanMountInBg())
                {
                    if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && !CurrentMountSpeed(bot))
                        ai->TellMasterNoFacing("Mount in bg. No attackers or far from target and not in combat.");

                    return Mount();
                }
            }
        }
    }

    //Unmounted when able to attack target
    if (canAttackTarget)
    {
        if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && CurrentMountSpeed(bot))
            ai->TellMasterNoFacing("Unmount. Able to attack target.");
        return UnMount();
    }

    //No more logic needed in bg.
    if (bot->InBattleGround())
        return false;

    //Chase to current target.
    if (!canAttackTarget && (farFromTarget || shouldChaseTarget))
    {
        if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && !CurrentMountSpeed(bot))
            ai->TellMasterNoFacing("Mount. Unable to attack target and target is far or chasable.");
        return Mount();
    }

    //Following master and close to master that is unmounted.
    if (ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT) && groupMaster && groupMaster != bot && !farFromMaster && !CurrentMountSpeed(groupMaster))
    {
        if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && CurrentMountSpeed(bot))
            ai->TellMasterNoFacing("Unmount. Near umounted group master.");
        return UnMount();
    }

    //Near guard position
    if (ai->HasStrategy("guard", ai->GetState()))
    {
        PositionMap& posMap = AI_VALUE(PositionMap&, "position");
        PositionEntry guardPosition = posMap["guard"];

        float distance = AI_VALUE2(float, "distance", "position_guard");

        if (guardPosition.isSet() && distance < ai->GetRange("follow"))
        {
            if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && CurrentMountSpeed(bot))
                ai->TellMasterNoFacing("Unmount. Near umounted guard position.");
            return UnMount();
        }
    }

    //Near stay position
    if (ai->HasStrategy("stay", ai->GetState()))
    {
        PositionMap& posMap = AI_VALUE(PositionMap&, "position");
        PositionEntry stayPosition = posMap["stay"];

        float distance = AI_VALUE2(float, "distance", "position_stay");

        if (stayPosition.isSet() && distance < ai->GetRange("follow"))
        {
            if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && CurrentMountSpeed(bot))
                ai->TellMasterNoFacing("Unmount. Near stay location.");
            return UnMount();
        }
    }

    //Doing stuff nearby.
    if (travelTarget->isWorking())
    {
        if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && CurrentMountSpeed(bot))
            ai->TellMasterNoFacing("Unmount. Near travel target.");
        return UnMount();
    }

    //Rping nearby.
    if (AI_VALUE(GuidPosition, "rpg target") && sServerFacade.IsDistanceLessThan(AI_VALUE2(float, "distance", "rpg target"), sPlayerbotAIConfig.farDistance))
    {
        if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && CurrentMountSpeed(bot))
            ai->TellMasterNoFacing("Unmount. Near rpg target.");
        return UnMount();
    }

    if (ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT) && groupMaster)
    {
        //Mounting with master.
        if (CurrentMountSpeed(groupMaster) && !hasAttackers)
        {
            if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && !CurrentMountSpeed(bot))
                ai->TellMasterNoFacing("Mount. Group master mounted and no attackers.");
            return Mount();
        }

        //Mounting to move to master.
        if (farFromMaster && !bot->IsInCombat())
        {
            if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && !CurrentMountSpeed(bot))
                ai->TellMasterNoFacing("Mount. Far from group master and not in combat.");
            return Mount();
        }
    }

    if (!ai->IsStateActive(BotState::BOT_STATE_COMBAT) && !hasEnemy)
    {
        //Mounting to travel.
        if (travelTarget->isTraveling() && AI_VALUE(bool, "can move around"))
        {
            if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && !CurrentMountSpeed(bot))
                ai->TellMasterNoFacing("Mount. Traveling some place.");
            return Mount();
        }
        else if (!hasAttackers)
        {
            //Mounting to move to rpg target.
            if (AI_VALUE(GuidPosition, "rpg target") && sServerFacade.IsDistanceGreaterThan(AI_VALUE2(float, "distance", "rpg target"), sPlayerbotAIConfig.sightDistance))
            {
                if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && !CurrentMountSpeed(bot))
                    ai->TellMasterNoFacing("Mount. Rpg target far away.");
                return Mount();
            }

            //Mounting in safe place.
            if (!ai->HasStrategy("guard", ai->GetState()) && !ai->HasStrategy("stay", ai->GetState()) && !AI_VALUE(list<ObjectGuid>, "possible rpg targets").empty() && urand(0, 100) > 50)
            {
                if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && !CurrentMountSpeed(bot))
                    ai->TellMasterNoFacing("Mount. Near rpg targets.");
                return Mount();
            }
        }

        //Far from guard position
        if (ai->HasStrategy("guard", ai->GetState()))
        {
            PositionMap& posMap = AI_VALUE(PositionMap&, "position");
            PositionEntry guardPosition = posMap["guard"];

            float distance = AI_VALUE2(float, "distance", "position_guard");

            if (guardPosition.isSet() && distance > 40.0f)
            {
                if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && !CurrentMountSpeed(bot))
                    ai->TellMasterNoFacing("Mount. Move to guard.");
                return Mount();
            }
        }

        //Far from stay position
        if (ai->HasStrategy("stay", ai->GetState()))
        {
            PositionMap& posMap = AI_VALUE(PositionMap&, "position");
            PositionEntry guardPosition = posMap["stay"];

            float distance = AI_VALUE2(float, "distance", "position_stay");

            if (guardPosition.isSet() && distance > 40.0f)
            {
                if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && !CurrentMountSpeed(bot))
                    ai->TellMasterNoFacing("Mount. Move to stay.");
                return Mount();
            }
        }
    }

    return false;
}

bool CheckMountStateAction::isUseful()
{
    // do not use on vehicle
    if (ai->IsInVehicle())
        return false;

    if (bot->IsDead())
        return false;

    bool isOutdoor = bot->GetMap()->GetTerrain()->IsOutdoors(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
    if (!isOutdoor)
        return false;

    if (bot->IsTaxiFlying())
        return false;

    if (bot->getClass() == CLASS_ROGUE && bot->InBattleGround() && (ai->HasAura("stealth", bot) || ai->HasAura("sprint", bot)))
        return false;

    if (bot->getClass() == CLASS_DRUID && bot->InBattleGround() && (ai->HasAura("prowl", bot) || ai->HasAura("dash", bot)))
        return false;

#ifndef MANGOSBOT_ZERO
    if (bot->InArena())
        return false;
#endif

    if (!bot->GetPlayerbotAI()->HasStrategy("mount", BotState::BOT_STATE_NON_COMBAT) && !bot->IsMounted())
        return false;

    bool firstmount = bot->GetLevel() >=
#ifdef MANGOSBOT_ZERO
        40
#else
#ifdef MANGOSBOT_ONE
        30
#else
        20
#endif
#endif
        ;
    if (!firstmount)
        return false;

    // Do not use with BG Flags
    if (bot->HasAura(23333) || bot->HasAura(23335) || bot->HasAura(34976))
    {
        return false;
    }

    // Only mount if BG starts in less than 30 sec
    if (bot->InBattleGround())
    {
        BattleGround* bg = bot->GetBattleGround();
        if (bg && bg->GetStatus() == STATUS_WAIT_JOIN)
        {
            if (bg->GetStartDelayTime() > BG_START_DELAY_30S)
                return false;
        }
    }

    if (!bot->GetMap()->IsMountAllowed())
        return false;

    if (GetBestMountSpells(false).empty() && GetBestMounts(false).empty() && GetBestMountSpells(true).empty() && GetBestMounts(true).empty())
        return false;

    return true;
}

bool CheckMountStateAction::CanFly() const
{
    if (bot->GetMapId() != 530 && bot->GetMapId() != 571)
        return false;

#ifdef MANGOSBOT_ONE
    uint32 zone, area;
    bot->GetZoneAndAreaId(zone, area);
    uint32 v_map = GetVirtualMapForMapAndZone(bot->GetMapId(), zone);
    MapEntry const* mapEntry = sMapStore.LookupEntry(v_map);
    if (!mapEntry || mapEntry->addon < 1 || !mapEntry->IsContinent())
        return false;
#endif
#ifdef MANGOSBOT_TWO
    uint32 zone, area;
    bot->GetZoneAndAreaId(zone, area);
    if (!bot->CanStartFlyInArea(bot->GetMapId(), zone, area, false))
        return false;
#endif

    if (GetBestMountSpells(true).empty() && GetBestMounts(true).empty())
        return false;

    return true;
}

bool CheckMountStateAction::CanMountInBg() const
{
    //Do not mount with or near flag.
    if (bot->GetBattleGroundTypeId() == BattleGroundTypeId::BATTLEGROUND_WS)
    {
        BattleGroundWS* bg = (BattleGroundWS*)ai->GetBot()->GetBattleGround();

        if (bot->HasAura(23333) || bot->HasAura(23335))
        {
            return false;
        }
#ifdef MANGOSBOT_ZERO
        //check near A Flag
        uint32 lowguid = 90000;
        uint32 id = 179830;
        GameObject* AllianceflagStand = bg->GetBgMap()->GetGameObject(ObjectGuid(HIGHGUID_GAMEOBJECT, id, lowguid));

        if (bot->IsWithinDistInMap(AllianceflagStand, 3.0f))
            return false;

        //check near H Flag
        lowguid = 90001;
        id = 179831;
        GameObject* HordeflagStand = bg->GetBgMap()->GetGameObject(ObjectGuid(HIGHGUID_GAMEOBJECT, id, lowguid));

        if (bot->IsWithinDistInMap(HordeflagStand, 3.0f))
            return false;
#endif
    }
    return true;
}

float CheckMountStateAction::GetAttackDistance() const
{
    switch (bot->getClass())
    {
    case CLASS_WARRIOR:
    case CLASS_PALADIN:
        return 10.0f;
    case CLASS_ROGUE:
        return 50.0f;
    }
    /*if (enemy)
    attack_distance /= 2;*/

    if (ai->IsHeal(bot) || ai->IsRanged(bot))
        return 40.0f;

    return 35.0f;
}

uint32 CheckMountStateAction::MountSpeed(const SpellEntry* const spellInfo, const bool canFly)
{
    bool isMount = false;

#ifdef MANGOSBOT_ZERO
    uint32 effect = SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED;
#else
    uint32 effect = canFly ? SPELL_AURA_MOD_FLIGHT_SPEED_MOUNTED : SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED; //If we can fly only look at flight speed. Normal mounts then don't get any speed.
#endif

    if (!spellInfo)
        return 0;

    switch (spellInfo->Id) //Aura's hard coded in spell.cpp
    {
    case 783:  //travel form
    case 2645: //ghost wolf
        if (!canFly)
            return 39;
        break;
    case 33943: //flight form
        if (canFly)
            return 59;
        break;
    case 40120: //swift flight form
        if (canFly)
            return 279;
        break;
    }

    for (int i = 0; i < 3; i++)
    {
        if (spellInfo->EffectApplyAuraName[i] == SPELL_AURA_MOUNTED)
        {
            isMount = true;
            break;
        }
    }

#ifndef MANGOSBOT_ZERO
    //This part stops bots from mounting flying mounts when they can't fly. This should be tweaked if bots ever are able to normally ride flying mounts in the old-world.
    if(isMount && !canFly)
    {
        for (int i = 0; i < 3; i++)
        {
            if (spellInfo->EffectApplyAuraName[i] == SPELL_AURA_MOD_FLIGHT_SPEED_MOUNTED)
            {
                return 0;
            }
        }
    }
#endif

    if (isMount)
    {
        for (int i = 0; i < 3; i++)
        {
            if (spellInfo->EffectApplyAuraName[i] == effect)
            {
                return spellInfo->EffectBasePoints[i];
            }
        }
    }

    return 0;
}

uint32 CheckMountStateAction::MountSpeed(const ItemPrototype* proto, const bool canFly)
{
    if (!proto)
        return 0;

    uint32 speed = 0;
    for (int j = 0; j < MAX_ITEM_PROTO_SPELLS; j++)
    {
        const SpellEntry* const spellInfo = sServerFacade.LookupSpellInfo(proto->Spells[j].SpellId);
        speed = MountSpeed(spellInfo, canFly);

        if (speed)
            return speed;
    }

    return 0;
}

vector<uint32> CheckMountStateAction::GetBestMountSpells(const bool canFly) const
{
    uint32 bestMountSpeed = 1;
    vector<uint32> spells;
    for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
    {
        uint32 spellId = itr->first;
        if (itr->second.state == PLAYERSPELL_REMOVED || itr->second.disabled || IsPassiveSpell(spellId))
            continue;

        const SpellEntry* spellInfo = sServerFacade.LookupSpellInfo(spellId);

        uint32 mountSpeed = MountSpeed(spellInfo, canFly);

        if (mountSpeed < bestMountSpeed)
            continue;

        if (mountSpeed > bestMountSpeed)
            spells.clear();

        spells.push_back(spellId);
    }

    return spells;
}

vector<Item*> CheckMountStateAction::GetBestMounts(const bool canFly) const
{
    list<Item*> items = AI_VALUE2(list<Item*>, "inventory items", "mount");

    uint32 bestMountSpeed = 1;
    vector<Item*> mounts;

    for (auto& item : items)
    {
        uint32 mountSpeed = MountSpeed(item->GetProto(), canFly);

        if (mountSpeed < bestMountSpeed)
            continue;

        if (mountSpeed > bestMountSpeed)
            mounts.clear();

        mounts.push_back(item);
    }

    return mounts;
}

uint32 CheckMountStateAction::GetBestMountSpeed(const bool canFly) const
{
    vector<uint32> mountSpells = GetBestMountSpells(canFly);
    vector<Item*> mounts = GetBestMounts(canFly);

    if (mountSpells.empty() && mounts.empty())
        return 0;

    SpellEntry const* mountSpell = nullptr;
    ItemPrototype const* mountItemProto = nullptr;

    if (!mountSpells.empty())
        mountSpell = sServerFacade.LookupSpellInfo(mountSpells.front());

    if (!mounts.empty())
        mountItemProto = mounts.front()->GetProto();

    return std::max(MountSpeed(mountSpell, canFly), MountSpeed(mountItemProto, canFly));
}

uint32 CheckMountStateAction::CurrentMountSpeed(const Unit* unit)
{
    uint32 mountSpeed = 0;

    for (uint32 auraType = SPELL_AURA_BIND_SIGHT; auraType < TOTAL_AURAS; auraType++)
    {
        Unit::AuraList const& auras = unit->GetAurasByType((AuraType)auraType);

        if (auras.empty())
            continue;

        for (Unit::AuraList::const_iterator i = auras.begin(); i != auras.end(); i++)
        {
            Aura* aura = *i;
            if (!aura)
                continue;

            SpellEntry const* auraSpell = aura->GetSpellProto();

            uint32 auraSpeed = std::max(MountSpeed(auraSpell, false), MountSpeed(auraSpell, true));

            if (auraSpeed < mountSpeed)
                continue;

            mountSpeed = auraSpeed;
        }
    }

    return mountSpeed;
}

bool CheckMountStateAction::MountWithBestMount(const bool canFly)
{
    vector<uint32> mountSpells = GetBestMountSpells(canFly);
    vector<Item*> mounts = GetBestMounts(canFly);

    if (mountSpells.empty() && mounts.empty())
        return false;

    SpellEntry const* mountSpell;
    Item* mountItem;

    if (!mountSpells.empty())
        mountSpell = sServerFacade.LookupSpellInfo(mountSpells.front());

    if (!mounts.empty())
        mountItem = mounts.front();

    if (!bot->IsStopped())
    {
        ai->StopMoving();
    }  

    if (mounts.empty() || (!mountSpells.empty() && MountSpeed(mountSpell, canFly) > MountSpeed(mountItem->GetProto(), canFly)))
    {
        uint32 spellId = mountSpells[urand(0, mountSpells.size() - 1)];
        if (ai->CastSpell(spellId, bot))
        {
            sPlayerbotAIConfig.logEvent(ai, "CheckMountStateAction", sServerFacade.LookupSpellInfo(mountSpells.front())->SpellName[0], to_string(GetBestMountSpeed(canFly)));
            SetDuration(GetSpellRecoveryTime(sServerFacade.LookupSpellInfo(mountSpells.front())));
            return true;
        }        
        return false;
    }
    else
    {
        Item* mount = mounts[urand(0, mounts.size() - 1)];
        if (UseItemAuto(mount))
        {
            sPlayerbotAIConfig.logEvent(ai, "CheckMountStateAction", mount->GetProto()->Name1, to_string(GetBestMountSpeed(canFly)));
            SetDuration(3000U); // 3s
            return true;
        }
    }

    return false;
}

bool CheckMountStateAction::Mount()
{
    uint32 currentSpeed = CurrentMountSpeed(bot);

    if (currentSpeed) //Already mounted
    {
        if (CurrentMountSpeed(bot) < GetBestMountSpeed(CanFly())) //Dismount to speed up.
            return UnMount();
        else
            return false;
    }

    Player* master = GetMaster();
    ai->RemoveShapeshift();

    bool didMount = false;

    if (CanFly())
    {
       didMount = MountWithBestMount(true);
    }

    if (!didMount)
        didMount = MountWithBestMount();

    return didMount;
}

bool CheckMountStateAction::UnMount() const
{
    if (!CurrentMountSpeed(bot))
        return false;

    if (bot->IsFlying() && WorldPosition(bot).currentHeight() > 10.0f)
        return false;

    if (bot->IsMounted())
    {
        WorldPacket emptyPacket;
        bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);

        if (bot->IsFlying())
            bot->GetMotionMaster()->MoveFall();
    }
    else
        ai->RemoveShapeshift();
    return true;
}