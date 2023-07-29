#include "botpch.h"
#include "../../playerbot.h"
#include "DungeonTriggers.h"
#include "../values/PositionValue.h"
#include "ServerFacade.h"
#include "../AiObjectContext.h"
#include "../values/HazardsValue.h"
#include "../actions/MovementActions.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"

using namespace ai;

bool EnterDungeonTrigger::IsActive()
{
    // Don't trigger if strategy already set
    if (!ai->HasStrategy(dungeonStrategy, BotState::BOT_STATE_COMBAT))
    {
        // If the bot is ready
        if (bot->IsInWorld() && !bot->IsBeingTeleported())
        {
            // If the bot is on the specified dungeon
            Map* map = bot->GetMap();
            if (map && (map->IsDungeon() || map->IsRaid()))
            {
                return map->GetId() == mapID;
            }
        }
    }

    return false;
}

bool LeaveDungeonTrigger::IsActive()
{
    // Don't trigger if strategy already unset
    if (ai->HasStrategy(dungeonStrategy, BotState::BOT_STATE_COMBAT))
    {
        // If the bot is ready
        if (bot->IsInWorld() && !bot->IsBeingTeleported())
        {
            // If the bot is not on the specified dungeon
            return bot->GetMapId() != mapID;
        }
    }

    return false;
}

bool StartBossFightTrigger::IsActive()
{
    // Don't trigger if strategy already set
    if (!ai->HasStrategy(bossStrategy, BotState::BOT_STATE_COMBAT))
    {
        // If the bot is ready
        if (bot->IsInWorld() && !bot->IsBeingTeleported())
        {
            AiObjectContext* context = ai->GetAiObjectContext();
            const list<ObjectGuid> attackers = AI_VALUE(list<ObjectGuid>, "attackers");
            for (const ObjectGuid& attackerGuid : attackers)
            {
                Unit* attacker = ai->GetUnit(attackerGuid);
                if (attacker && attacker->GetEntry() == bossID)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool EndBossFightTrigger::IsActive()
{
    // Don't trigger if strategy already unset
    if (ai->HasStrategy(bossStrategy, BotState::BOT_STATE_COMBAT))
    {
        // We consider the fight is over if not in combat
        return !ai->IsStateActive(BotState::BOT_STATE_COMBAT);
    }

    return false;
}

bool CloseToGameObjectHazardTrigger::IsActive()
{
    // If the bot is ready
    bool closeToHazard = false;
    if (bot->IsInWorld() && !bot->IsBeingTeleported())
    {
        AiObjectContext* context = ai->GetAiObjectContext();

        // This has a maximum range equal to the sight distance on config file (default 60 yards)
        const list<ObjectGuid>& gameObjects = AI_VALUE2(list<ObjectGuid>, "nearest game objects no los", gameObjectID);
        for (const ObjectGuid& gameObjectGuid : gameObjects)
        {
            GameObject* gameObject = ai->GetGameObject(gameObjectGuid);
            if (gameObject)
            {
                const float distance = bot->GetDistance(gameObject) + gameObject->GetObjectBoundingRadius();
                if (distance <= radius)
                {
                    closeToHazard = true;
                }

                // Cache the hazards
                Hazard hazard(gameObjectGuid, expirationTime, radius);
                SET_AI_VALUE(Hazard, "add hazard", std::move(hazard));
            }
        }
    }

    // Don't trigger if the bot is moving
    if (closeToHazard)
    {
        const Action* lastExecutedAction = ai->GetLastExecutedAction(BotState::BOT_STATE_COMBAT);
        if (lastExecutedAction)
        {
            const MovementAction* movementAction = dynamic_cast<const MovementAction*>(lastExecutedAction);
            if (movementAction)
            {
                closeToHazard = false;
            }
        }
    }

    return closeToHazard;
}

bool CloseToCreatureTrigger::IsActive()
{
    // If the bot is ready
    if (bot->IsInWorld() && !bot->IsBeingTeleported())
    {
        AiObjectContext* context = ai->GetAiObjectContext();

        // Iterate through the near creatures
        list<Unit*> creatures;
        MaNGOS::AllCreaturesOfEntryInRangeCheck u_check(bot, creatureID, range);
        MaNGOS::UnitListSearcher<MaNGOS::AllCreaturesOfEntryInRangeCheck> searcher(creatures, u_check);
        Cell::VisitAllObjects(bot, searcher, range);
        for (Unit* unit : creatures)
        {
            Creature* creature = (Creature*)unit;
            if (creature)
            {
                // Check if the bot is not being targeted by the creature
                if (!creature->GetVictim() || (creature->GetVictim()->GetObjectGuid() != bot->GetObjectGuid()))
                {
                    // See if the creature is within the specified distance
                    if (bot->IsWithinDist(creature, range))
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool ItemReadyTrigger::IsActive()
{
    // Check if the bot has the item or if it has cheats enabled
    if (bot->HasItemCount(itemID, 1) || ai->HasCheat(BotCheatMask::item))
    {
        const ItemPrototype* proto = sObjectMgr.GetItemPrototype(itemID);
        if (proto)
        {
            // Check if the item is in cooldown
            bool inCooldown = false;
            for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
            {
                if (proto->Spells[i].SpellTrigger != ITEM_SPELLTRIGGER_ON_USE)
                {
                    continue;
                }

                const uint32 spellID = proto->Spells[i].SpellId;
                if (spellID > 0)
                {
                    if (!sServerFacade.IsSpellReady(bot, spellID) ||
                        !sServerFacade.IsSpellReady(bot, spellID, itemID))
                    {
                        inCooldown = true;
                        break;
                    }
                }
            }

            return !inCooldown;
        }
    }

    return false;
}

bool ItemBuffReadyTrigger::IsActive()
{
    if (!bot->HasAura(buffID))
    {
        return ItemReadyTrigger::IsActive();
    }

    return false;
}

bool BuffOnTargetTrigger::IsActive()
{
    const Unit* target = GetTarget();
    return target && target->HasAura(buffID);
}
