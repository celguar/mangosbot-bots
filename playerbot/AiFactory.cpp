#include "../botpch.h"
#include "playerbot.h"
#include "AiFactory.h"
#include "strategy/Engine.h"

#include "strategy/priest/PriestAiObjectContext.h"
#include "strategy/mage/MageAiObjectContext.h"
#include "strategy/warlock/WarlockAiObjectContext.h"
#include "strategy/warrior/WarriorAiObjectContext.h"
#include "strategy/shaman/ShamanAiObjectContext.h"
#include "strategy/paladin/PaladinAiObjectContext.h"
#include "strategy/druid/DruidAiObjectContext.h"
#include "strategy/hunter/HunterAiObjectContext.h"
#include "strategy/rogue/RogueAiObjectContext.h"
#include "strategy/deathknight/DKAiObjectContext.h"
#include "Player.h"
#include "PlayerbotAIConfig.h"
#include "RandomPlayerbotMgr.h"
#include "BattleGroundMgr.h"


AiObjectContext* AiFactory::createAiObjectContext(Player* player, PlayerbotAI* ai)
{
    switch (player->getClass())
    {
    case CLASS_PRIEST:
        return new PriestAiObjectContext(ai);
        break;
    case CLASS_MAGE:
        return new MageAiObjectContext(ai);
        break;
    case CLASS_WARLOCK:
        return new WarlockAiObjectContext(ai);
        break;
    case CLASS_WARRIOR:
        return new WarriorAiObjectContext(ai);
        break;
    case CLASS_SHAMAN:
        return new ShamanAiObjectContext(ai);
        break;
    case CLASS_PALADIN:
        return new PaladinAiObjectContext(ai);
        break;
    case CLASS_DRUID:
        return new DruidAiObjectContext(ai);
        break;
    case CLASS_HUNTER:
        return new HunterAiObjectContext(ai);
        break;
    case CLASS_ROGUE:
        return new RogueAiObjectContext(ai);
        break;
#ifdef MANGOSBOT_TWO
    case CLASS_DEATH_KNIGHT:
        return new DKAiObjectContext(ai);
        break;
#endif
    }
    return new AiObjectContext(ai);
}

int AiFactory::GetPlayerSpecTab(Player* bot)
{
    map<uint32, int32> tabs = GetPlayerSpecTabs(bot);

    if (bot->GetLevel() >= 10 && ((tabs[0] + tabs[1] + tabs[2]) > 0))
    {
        int tab = -1, max = 0;
        for (uint32 i = 0; i < uint32(3); i++)
        {
            if (tab == -1 || max < tabs[i])
            {
                tab = i;
                max = tabs[i];
            }
        }
        return tab;
    }
    else
    {
        int tab = 0;

        switch (bot->getClass())
        {
        case CLASS_MAGE:
            tab = 1;
            break;
        case CLASS_PALADIN:
            tab = 0;
            break;
        case CLASS_PRIEST:
            tab = 1;
            break;
        }
        return tab;
    }
}

map<uint32, int32> AiFactory::GetPlayerSpecTabs(Player* bot)
{
    map<uint32, int32> tabs;
    for (uint32 i = 0; i < uint32(3); i++)
        tabs[i] = 0;

    uint32 classMask = bot->getClassMask();
    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const *talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo)
            continue;

        TalentTabEntry const *talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);
        if (!talentTabInfo)
            continue;

        if ((classMask & talentTabInfo->ClassMask) == 0)
            continue;

        int maxRank = 0;
        for (int rank = MAX_TALENT_RANK - 1; rank >= 0; --rank)
        {
            if (!talentInfo->RankID[rank])
                continue;

            uint32 spellid = talentInfo->RankID[rank];
            if (spellid && bot->HasSpell(spellid))
                maxRank = rank + 1;

        }
        tabs[talentTabInfo->tabpage] += maxRank;
    }

    return tabs;
}

BotRoles AiFactory::GetPlayerRoles(Player* player)
{
    BotRoles role = BOT_ROLE_NONE;
    int tab = GetPlayerSpecTab(player);
    switch (player->getClass())
    {
        case CLASS_PRIEST:
            if (tab == 2)
                role = BOT_ROLE_DPS;
            else
                role = BOT_ROLE_HEALER;
            break;
        case CLASS_SHAMAN:
            if (tab == 2)
                role = BOT_ROLE_HEALER;
            else
                role = BOT_ROLE_DPS;
            break;
        case CLASS_WARRIOR:
            if (tab == 2)
                role = BOT_ROLE_TANK;
            else
                role = BOT_ROLE_DPS;
            break;
        case CLASS_PALADIN:
            if (tab == 0)
                role = BOT_ROLE_HEALER;
            else if (tab == 1)
                role = BOT_ROLE_TANK;
            else if (tab == 2)
                role = BOT_ROLE_DPS;
            break;
        case CLASS_DRUID:
            if (tab == 0)
                role = BOT_ROLE_DPS;
            else if (tab == 1)
                role = (BotRoles)(BOT_ROLE_TANK | BOT_ROLE_DPS);
            else if (tab == 2)
                role = BOT_ROLE_HEALER;
            break;
        default:
            role = BOT_ROLE_DPS;
            break;
    }
    return role;
}

void AiFactory::AddDefaultCombatStrategies(Player* player, PlayerbotAI* const facade, Engine* engine)
{
    int tab = GetPlayerSpecTab(player);

    if (!player->InBattleGround())
    {
        engine->addStrategies("racials", "chat", "default", "potions", "cast time", "duel", "pvp");
    }

    switch (player->getClass())
    {
        case CLASS_PRIEST:
            if (tab == 2)
            {
                engine->addStrategies("dps", "shadow debuff", "shadow aoe", "threat");
            }
            else if (tab == 0)
            {
                engine->addStrategies("holy", "shadow debuff", "shadow aoe", "threat");
                    //if (player->GetLevel() >= 4)
                       //engine->addStrategy("dps debuff");
            }
            else
                engine->addStrategies("heal", "threat");

            engine->addStrategies("dps assist", "flee", "cure", "ranged", "cc");
            break;
        case CLASS_MAGE:
            if (tab == 0)
                engine->addStrategies("arcane", "threat");
            else if (tab == 1)
                engine->addStrategies("fire", "fire aoe", "threat");
            else
                engine->addStrategies("frost", "frost aoe", "threat", "dps aoe");

            engine->addStrategies("dps", "dps assist", "flee", "cure", "ranged", "cc");
            break;
        case CLASS_WARRIOR:
            if (tab == 2)
                engine->addStrategies("tank", "tank assist", "aoe", "close", "mark rti");
            else if (player->GetLevel() < 30 || tab == 0)
                engine->addStrategies("arms", "aoe", "dps assist", "threat", "close", "behind");
            else
                engine->addStrategies("fury", "aoe", "dps assist", "threat", "close", "behind");
            break;
        case CLASS_SHAMAN:
            if (tab == 0)
                engine->addStrategies("caster", "caster aoe", "bmana", "threat", "flee", "ranged");
            else if (tab == 2)
                engine->addStrategies("heal", "bmana", "flee", "ranged");
            else
                engine->addStrategies("dps", "melee aoe", "bdps", "threat", "close");

            engine->addStrategies("dps assist", "cure", "totems");
            break;
        case CLASS_PALADIN:
            if (tab == 1)
                engine->addStrategies("tank", "tank assist", "bthreat", "cure", "close", "cc");
			else if(tab == 0)
                engine->addStrategies("heal", "dps assist", "cure", "flee", "cc", "ranged");
            else
                engine->addStrategies("dps", "dps assist", "cure", "close", "cc");

            /*if (player->GetLevel() < 14)
            {
                engine->addStrategy("bdps");
            }
            if (player->GetLevel() < 16)
            {
                engine->addStrategy("barmor");
            }*/
            break;
        case CLASS_DRUID:
            if (tab == 0)
            {
                engine->addStrategies("caster", "cure", "caster aoe", "threat", "flee", "dps assist", "ranged", "cc");
                if (player->GetLevel() > 19)
                    engine->addStrategy("caster debuff");
            }
            else if (tab == 2)
                engine->addStrategies("heal", "cure", "flee", "dps assist", "ranged", "cc");
            else
            {
                engine->removeStrategy("ranged");
                engine->addStrategies("bear", "tank assist", "flee", "close", "behind");
            }
            break;
        case CLASS_HUNTER:
            engine->addStrategies("dps", "bdps", "threat", "dps assist", "ranged", "cc", "aoe");
            if (player->GetLevel() > 19)
                engine->addStrategy("dps debuff");
            break;
        case CLASS_ROGUE:
            if (tab == 0)
                engine->addStrategies("assassin", "threat", "dps assist", "aoe", "close", "cc", "behind", "stealth");
            else if (tab == 1)
                engine->addStrategies("combat", "threat", "dps assist", "aoe", "close", "cc", "behind", "stealth");
            else
                engine->addStrategies("subtlety", "threat", "dps assist", "aoe", "close", "cc", "behind", "stealth");

            if (player->GetLevel() < 15)
                engine->addStrategies("dps");
            break;
        case CLASS_WARLOCK:
            if (player->GetLevel() > 19)
                engine->addStrategy("dps debuff");

            engine->addStrategies("dps assist", "dps", "flee", "ranged", "cc", "pet", "threat", "aoe");
            break;
#ifdef MANGOSBOT_TWO
        case CLASS_DEATH_KNIGHT:
            if (tab == 0)
                engine->addStrategies("blood");
            else if (tab == 1)
                engine->addStrategies("frost", "frost aoe", "dps assist", "threat");
            else
                engine->addStrategies("unholy", "unholy aoe", "dps assist", "threat");

            engine->addStrategies("dps assist", "flee", "close", "cc");
            break;
#endif
    }

	if (facade->IsRealPlayer() || sRandomPlayerbotMgr.IsRandomBot(player))
	{
        if (!player->GetGroup())
        {
            engine->addStrategy("flee");
            engine->addStrategy("boost");

            
            if (player->getClass() == CLASS_DRUID && tab == 2)
            {
                engine->addStrategies("caster", "caster aoe");
            }

            if (player->getClass() == CLASS_DRUID && tab == 1 && urand(0, 100) > 50 && player->GetLevel() > 19)
            {
                engine->addStrategy("dps");
            }

            if (player->getClass() == CLASS_PRIEST && tab == 1)
            {
                engine->removeStrategy("heal");
                engine->addStrategies("holy", "shadow debuff", "shadow aoe", "threat");
            }

            if (player->getClass() == CLASS_SHAMAN && tab == 2)
            {
                engine->addStrategies("caster", "caster aoe");
            }

            if (player->getClass() == CLASS_PALADIN && tab == 0)
            {
                engine->removeStrategy("ranged");
                engine->addStrategies("dps", "close");
                engine->removeStrategy("ranged");
            }
        }

        // enable paladin fight at low level
        if (player->getClass() == CLASS_PALADIN && tab == 0 && player->GetLevel() < 10)
        {
            engine->removeStrategy("ranged");
            engine->addStrategies("dps", "close");
            engine->removeStrategy("ranged");
        }

        // remove threat for now
        engine->removeStrategy("threat");

        engine->ChangeStrategy(sPlayerbotAIConfig.randomBotCombatStrategies);
    }
    else
    {
        engine->ChangeStrategy(sPlayerbotAIConfig.combatStrategies);
    }

    // Battleground switch
    if (player->InBattleGround())
    {
        BattleGroundTypeId bgType = player->GetBattleGroundTypeId();
#ifdef MANGOSBOT_TWO
        if (bgType == BATTLEGROUND_RB)
            bgType = player->GetBattleGround()->GetTypeId(true);

        if (bgType == BATTLEGROUND_IC)
            engine->addStrategy("isle");
#endif
        if (bgType == BATTLEGROUND_WS)
            engine->addStrategy("warsong");

        if (bgType == BATTLEGROUND_AB)
            engine->addStrategy("arathi");

        if(bgType == BATTLEGROUND_AV)
            engine->addStrategy("alterac");

#ifndef MANGOSBOT_ZERO
        if (bgType == BATTLEGROUND_EY)
            engine->addStrategy("eye");
#endif

#ifndef MANGOSBOT_ZERO
        if (player->InArena())
        {
            engine->addStrategy("arena");
        }
#endif
        engine->addStrategies("boost", "racials", "chat", "default", "aoe", "potions", "conserve mana", "cast time", "dps assist", "pvp");
        engine->removeStrategy("custom::say");
        engine->removeStrategy("flee");
        engine->removeStrategy("threat");
        engine->addStrategy("boost");

        if ((player->getClass() == CLASS_DRUID && tab == 2) || (player->getClass() == CLASS_SHAMAN && tab == 2))
            engine->addStrategies("caster", "caster aoe");

        if (player->getClass() == CLASS_DRUID && tab == 1)
            engine->addStrategies("behind", "dps");
        
        if (player->getClass() == CLASS_ROGUE)
            engine->addStrategies("behind", "stealth");
    }
}

Engine* AiFactory::createCombatEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* AiObjectContext) {
	Engine* engine = new Engine(facade, AiObjectContext);
    AddDefaultCombatStrategies(player, facade, engine);
    return engine;
}

void AiFactory::AddDefaultNonCombatStrategies(Player* player, PlayerbotAI* const facade, Engine* nonCombatEngine)
{
    int tab = GetPlayerSpecTab(player);

    switch (player->getClass()){
        case CLASS_PRIEST:
            nonCombatEngine->addStrategies("dps assist", "cure", "crusader");
            break;
        case CLASS_PALADIN:
            if (tab == 1)
                nonCombatEngine->addStrategies("bthreat", "tank assist", "bkings", "baoe");
            else if (tab == 0)
                nonCombatEngine->addStrategies("dps assist", "bwisdom", "bconcentration");
            else
                nonCombatEngine->addStrategies("dps assist", "bmight", "bsanctity");

            /*if (player->GetLevel() < 14)
                nonCombatEngine->addStrategies("bdps");
            if (player->GetLevel() < 16)
                nonCombatEngine->addStrategies("barmor");*/

            nonCombatEngine->addStrategies("cure");
            break;
        case CLASS_HUNTER:
            nonCombatEngine->addStrategies("bdps", "dps assist", "pet");
            break;
        case CLASS_SHAMAN:
            if (tab == 0 || tab == 2)
                nonCombatEngine->addStrategy("bmana");
            else
                nonCombatEngine->addStrategy("bdps");

            nonCombatEngine->addStrategies("dps assist", "cure");
            break;
        case CLASS_MAGE:
            if (tab == 1)
                nonCombatEngine->addStrategy("bdps");
            else
                nonCombatEngine->addStrategy("bmana");

            nonCombatEngine->addStrategies("dps assist", "cure");
            break;
        case CLASS_DRUID:
            if (tab == 1)
                nonCombatEngine->addStrategies("tank assist", "cure");
            else
                nonCombatEngine->addStrategies("dps assist", "cure");
            break;
        case CLASS_WARRIOR:
            if (tab == 2)
                nonCombatEngine->addStrategy("tank assist");
            else
                nonCombatEngine->addStrategy("dps assist");
            break;
        case CLASS_WARLOCK:
            nonCombatEngine->addStrategies("pet", "dps assist");
            break;
#ifdef MANGOSBOT_TWO
        case CLASS_DEATH_KNIGHT:
            if (tab == 0)
                nonCombatEngine->addStrategy("tank assist");
            else
                nonCombatEngine->addStrategy("dps assist");
            break;
#endif
        default:
            nonCombatEngine->addStrategy("dps assist");
            break;
    }

    if (!player->InBattleGround())
    {
        nonCombatEngine->addStrategies("racials", "nc", "food", "chat", "follow",
            "default", "quest", "loot", "gather", "duel", "emote", "buff", "mount");
    }

    if ((facade->IsRealPlayer() || sRandomPlayerbotMgr.IsRandomBot(player)) && !player->InBattleGround())
    {   
        Player* master = facade->GetMaster();

        // let 25% of free bots start duels.
        if (!urand(0, 3))
            nonCombatEngine->addStrategy("start duel");

        if (sPlayerbotAIConfig.randomBotJoinLfg)
            nonCombatEngine->addStrategy("lfg");

        if (!player->GetGroup() || player->GetGroup()->GetLeaderGuid() == player->GetObjectGuid())
        {
            // let 25% of random not grouped (or grp leader) bots help other players
            if (!urand(0, 3))
                nonCombatEngine->addStrategy("attack tagged");

            nonCombatEngine->addStrategy("pvp");
            nonCombatEngine->addStrategy("collision");
            nonCombatEngine->addStrategy("grind");            
            nonCombatEngine->addStrategy("group");
            nonCombatEngine->addStrategy("guild");

            if (sPlayerbotAIConfig.autoDoQuests)
            {
                nonCombatEngine->addStrategy("travel");
                nonCombatEngine->addStrategy("rpg");                
            }

            if (sPlayerbotAIConfig.randomBotJoinBG)
                nonCombatEngine->addStrategy("bg");

            if(!master || master->GetPlayerbotAI())
                nonCombatEngine->addStrategy("maintenance");


            nonCombatEngine->ChangeStrategy(sPlayerbotAIConfig.randomBotNonCombatStrategies);
        }
        else {
            if (facade)
            {
                if (master)
                {
                    if (master->GetPlayerbotAI() || sRandomPlayerbotMgr.IsRandomBot(player))
                    {
                        nonCombatEngine->addStrategy("pvp");
                        nonCombatEngine->addStrategy("collision");
                        nonCombatEngine->addStrategy("grind");
                        nonCombatEngine->addStrategy("group");
                        nonCombatEngine->addStrategy("guild");

                        if (sPlayerbotAIConfig.autoDoQuests)
                        {
                            nonCombatEngine->addStrategy("travel");
                            nonCombatEngine->addStrategy("rpg");
                        }

                        if (!master || master->GetPlayerbotAI())
                            nonCombatEngine->addStrategy("maintenance");

                        nonCombatEngine->ChangeStrategy(sPlayerbotAIConfig.randomBotNonCombatStrategies);
                    }
                    else
                        nonCombatEngine->ChangeStrategy(sPlayerbotAIConfig.nonCombatStrategies);

                }
            }
        }
    }
    else
    {
        nonCombatEngine->ChangeStrategy(sPlayerbotAIConfig.nonCombatStrategies);
    }

    // Battleground switch
    if (player->InBattleGround())
    {
        nonCombatEngine->addStrategies("racials", "nc", "chat",
            "default", "buff", "food", "mount", "pvp", "collision", "dps assist", "attack tagged", "emote");
        nonCombatEngine->removeStrategy("custom::say");
        nonCombatEngine->removeStrategy("travel");
        nonCombatEngine->removeStrategy("rpg");
        nonCombatEngine->removeStrategy("grind");

        BattleGroundTypeId bgType = player->GetBattleGroundTypeId();
#ifdef MANGOSBOT_TWO
        if (bgType == BATTLEGROUND_RB)
            bgType = player->GetBattleGround()->GetTypeId(true);
#endif

        bool isArena = false;

#ifndef MANGOSBOT_ZERO
        if (player->InArena())
            isArena = true;
#endif
        if (isArena)
        {
            nonCombatEngine->addStrategy("arena");
            nonCombatEngine->removeStrategy("mount");
        }
        else
        {
#ifndef MANGOSBOT_ZERO
#ifdef MANGOSBOT_TWO
            if (bgType <= BATTLEGROUND_EY || bgType == BATTLEGROUND_IC) // do not add for not supported bg
                nonCombatEngine->addStrategy("battleground");
#else
            if (bgType <= BATTLEGROUND_EY) // do not add for not supported bg
                nonCombatEngine->addStrategy("battleground");
#endif
#else
            if (bgType <= BATTLEGROUND_AB) // do not add for not supported bg
                nonCombatEngine->addStrategy("battleground");
#endif

            if (bgType == BATTLEGROUND_WS)
                nonCombatEngine->addStrategy("warsong");

            if (bgType == BATTLEGROUND_AV)
                nonCombatEngine->addStrategy("alterac");

            if (bgType == BATTLEGROUND_AB)
                nonCombatEngine->addStrategy("arathi");

#ifndef MANGOSBOT_ZERO
            if (bgType == BATTLEGROUND_EY)
                nonCombatEngine->addStrategy("eye");
#endif
#ifdef MANGOSBOT_TWO
            if (bgType == BATTLEGROUND_IC)
                nonCombatEngine->addStrategy("isle");
#endif
        }
    }
}

Engine* AiFactory::createNonCombatEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* AiObjectContext) {
	Engine* nonCombatEngine = new Engine(facade, AiObjectContext);

    AddDefaultNonCombatStrategies(player, facade, nonCombatEngine);
	return nonCombatEngine;
}

void AiFactory::AddDefaultDeadStrategies(Player* player, PlayerbotAI* const facade, Engine* deadEngine)
{
    deadEngine->addStrategies("dead", "stay", "chat", "default", "follow");
    if (sRandomPlayerbotMgr.IsRandomBot(player) && !player->GetGroup())
    {
        deadEngine->removeStrategy("follow");
    }
}

Engine* AiFactory::createDeadEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* AiObjectContext) {
    Engine* deadEngine = new Engine(facade, AiObjectContext);
    AddDefaultDeadStrategies(player, facade, deadEngine);
    return deadEngine;
}
