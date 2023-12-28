#include "../botpch.h"
#include "playerbot.h"
#include "PlayerbotFactory.h"

#include "../ahbot/AhBotConfig.h"
#include "SQLStorages.h"
#include "ItemPrototype.h"
#include "PlayerbotAIConfig.h"
#include "AccountMgr.h"
#include "DBCStore.h"
#include "SharedDefines.h"
#include "ahbot/AhBot.h"
#include "RandomItemMgr.h"
#include "RandomPlayerbotFactory.h"
#include "ServerFacade.h"
#include "AiFactory.h"

#ifndef MANGOSBOT_ZERO
    #ifdef CMANGOS
        #include "Arena/ArenaTeam.h"
    #endif
    #ifdef MANGOS
        #include "ArenaTeam.h"
    #endif
#endif
#include "strategy/ItemVisitors.h"

using namespace ai;
using namespace std;

#define PLAYER_SKILL_INDEX(x)       (PLAYER_SKILL_INFO_1_1 + ((x)*3))

uint32 PlayerbotFactory::tradeSkills[] =
{
    SKILL_ALCHEMY,
    SKILL_ENCHANTING,
    SKILL_SKINNING,
    SKILL_TAILORING,
    SKILL_LEATHERWORKING,
    SKILL_ENGINEERING,
    SKILL_HERBALISM,
    SKILL_MINING,
    SKILL_BLACKSMITHING,
    SKILL_COOKING,
    SKILL_FIRST_AID,
    SKILL_FISHING
#ifndef MANGOSBOT_ZERO
	,
	SKILL_JEWELCRAFTING
#endif
};

list<uint32> PlayerbotFactory::classQuestIds;
list<uint32> PlayerbotFactory::specialQuestIds;

TaxiNodeLevelContainer PlayerbotFactory::overworldTaxiNodeLevelsA;
TaxiNodeLevelContainer PlayerbotFactory::overworldTaxiNodeLevelsH;

void PlayerbotFactory::Init()
{
	if (sPlayerbotAIConfig.randomBotPreQuests) {
		ObjectMgr::QuestMap const& questTemplates = sObjectMgr.GetQuestTemplates();
		for (ObjectMgr::QuestMap::const_iterator i = questTemplates.begin(); i != questTemplates.end(); ++i)
		{
			uint32 questId = i->first;
			Quest const *quest = i->second;

			if (!quest->GetRequiredClasses() || quest->IsRepeatable() || quest->GetMinLevel() < 10)
				continue;

			AddPrevQuests(questId, classQuestIds);
			classQuestIds.remove(questId);
			classQuestIds.push_back(questId);
		}
        for (list<uint32>::iterator i = sPlayerbotAIConfig.randomBotQuestIds.begin(); i != sPlayerbotAIConfig.randomBotQuestIds.end(); ++i)
        {
            uint32 questId = *i;
            AddPrevQuests(questId, specialQuestIds);
            specialQuestIds.remove(questId);
            specialQuestIds.push_back(questId);
        }
	}

    for (uint32 i = 1; i < sTaxiNodesStore.GetNumRows(); ++i)
    {
        TaxiNodesEntry const* taxiNode = sTaxiNodesStore.LookupEntry(i);

        if (!taxiNode)
            continue;

        WorldPosition taxiPosition(taxiNode);

        if (!taxiPosition.isOverworld())
            continue;

        TaxiNodeLevel taxiNodeLevel = TaxiNodeLevel();

        taxiNodeLevel.Index = i;
        taxiNodeLevel.MapId = taxiNode->map_id;
        taxiNodeLevel.Level = taxiPosition.getAreaLevel();

        if (taxiNode->MountCreatureID[0])
            overworldTaxiNodeLevelsH.push_back(taxiNodeLevel);

        if (taxiNode->MountCreatureID[1])
            overworldTaxiNodeLevelsA.push_back(taxiNodeLevel);
    }
}

void PlayerbotFactory::Prepare()
{
    /*if (!itemQuality)
    {
        if (level < 20)
            itemQuality = urand(ITEM_QUALITY_POOR, ITEM_QUALITY_RARE);
        else if (level < 40)
            itemQuality = urand(ITEM_QUALITY_UNCOMMON, ITEM_QUALITY_RARE);
        else if (level < 60)
#ifdef MANGOSBOT_ZERO
            itemQuality = urand(ITEM_QUALITY_UNCOMMON, ITEM_QUALITY_EPIC);
		else if (level < 70)
			itemQuality = urand(ITEM_QUALITY_RARE, ITEM_QUALITY_EPIC);
#else
			itemQuality = urand(ITEM_QUALITY_UNCOMMON, ITEM_QUALITY_EPIC);
		else if (level < 70)
			itemQuality = urand(ITEM_QUALITY_UNCOMMON, ITEM_QUALITY_EPIC);
#endif
		else if (level < 80)
			itemQuality = urand(ITEM_QUALITY_RARE, ITEM_QUALITY_EPIC);
        else
            itemQuality = urand(ITEM_QUALITY_RARE, ITEM_QUALITY_EPIC);
    }*/

    if (sServerFacade.UnitIsDead(bot))
    {
        bot->ResurrectPlayer(1.0f, false);
        bot->SpawnCorpseBones();
    }

    bot->CombatStop(true);
    /*if (sPlayerbotAIConfig.disableRandomLevels)
    {
        if (bot->GetLevel() < sPlayerbotAIConfig.randombotStartingLevel)
        {
            bot->SetLevel(sPlayerbotAIConfig.randombotStartingLevel);
        }
    }*/

    if (!sPlayerbotAIConfig.disableRandomLevels)
    {
        bot->SetLevel(level);
    }

    if (!sPlayerbotAIConfig.randomBotShowHelmet)
    {
       bot->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_HELM);
    }

    if (!sPlayerbotAIConfig.randomBotShowCloak)
    {
       bot->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_CLOAK);
    }
}

void PlayerbotFactory::Randomize(bool incremental, bool syncWithMaster)
{
    sLog.outDetail("Preparing to %s randomize...", (incremental ? "incremental" : "full"));
    Prepare();

    if (sPlayerbotAIConfig.disableRandomLevels)
    {
        return;
    }
    bool isRealRandomBot = sRandomPlayerbotMgr.IsRandomBot(bot);
    bool isRandomBot = sRandomPlayerbotMgr.IsRandomBot(bot) && bot->GetPlayerbotAI() && !bot->GetPlayerbotAI()->HasRealPlayerMaster() && !bot->GetPlayerbotAI()->IsInRealGuild();

    sLog.outDetail("Resetting player...");
    PerformanceMonitorOperation* pmo = sPerformanceMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Reset");
    //ClearSkills();
    ClearSpells();

    if (!incremental && isRandomBot)
    {
        ClearInventory();
        ResetQuests();
        bot->resetTalents(true);
        CancelAuras();
    }
    if (isRealRandomBot)
    {
        InitQuests(specialQuestIds);
        bot->learnQuestRewardedSpells();
    }
    if (pmo) pmo->finish();

    pmo = sPerformanceMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Bags");
    sLog.outDetail("Initializing bags...");
    InitBags();
    if (pmo) pmo->finish();

    pmo = sPerformanceMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Spells1");
    sLog.outDetail("Initializing spells (step 1)...");
    InitAvailableSpells();
    if (pmo) pmo->finish();

    sLog.outDetail("Initializing skills (step 1)...");
    pmo = sPerformanceMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Skills1");
    InitSkills();
    InitTradeSkills();
    if (pmo) pmo->finish();

    pmo = sPerformanceMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Talents");
    sLog.outDetail("Initializing talents...");
    //InitTalentsTree(incremental);    
    //sRandomPlayerbotMgr.SetValue(bot->GetGUIDLow(), "specNo", 0);
    ai->DoSpecificAction("auto talents");

    if (!incremental && isRandomBot)
        sPlayerbotDbStore.Reset(ai);

    ai->ResetStrategies(incremental); // fix wrong stored strategy
    if (pmo) pmo->finish();

    pmo = sPerformanceMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Spells2");
    sLog.outDetail("Initializing spells (step 2)...");
    InitAvailableSpells();
    InitSpecialSpells();
    if (pmo) pmo->finish();

    if (isRealRandomBot)
    {
        pmo = sPerformanceMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Mounts");
        sLog.outDetail("Initializing mounts...");
        InitMounts();
        if (pmo) pmo->finish();
    }

    pmo = sPerformanceMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Skills2");
    sLog.outDetail("Initializing skills (step 2)...");
    UpdateTradeSkills();
    if (pmo) pmo->finish();

    if (isRealRandomBot)
    {
        pmo = sPerformanceMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Reputations");
        sLog.outDetail("Initializing reputations...");
        InitReputations();
        if (pmo) pmo->finish();
    }

    pmo = sPerformanceMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Equip");
    sLog.outDetail("Initializing equipmemt...");
    if (bot->GetLevel() >= sPlayerbotAIConfig.minEnchantingBotLevel)
    {
        sLog.outDetail("Initializing enchant templates...");
        LoadEnchantContainer();
    }

    InitEquipment(incremental, syncWithMaster);
    InitGems();
    if (pmo) pmo->finish();

    if (isRandomBot)
    {
        pmo = sPerformanceMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Ammo");
        sLog.outDetail("Initializing ammo...");
        InitAmmo();
        if (pmo) pmo->finish();

        pmo = sPerformanceMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Food");
        sLog.outDetail("Initializing food...");
        InitFood();
        if (pmo) pmo->finish();

        pmo = sPerformanceMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Potions");
        sLog.outDetail("Initializing potions...");
        InitPotions();
        if (pmo) pmo->finish();

        pmo = sPerformanceMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Reagents");
        sLog.outDetail("Initializing reagents...");
        InitReagents();
        if (pmo) pmo->finish();
    }

    if (!incremental)
    {
        pmo = sPerformanceMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Consumables");
        sLog.outDetail("Initializing consumables...");
        AddConsumables();
        if (pmo) pmo->finish();
    }

    if (!incremental && isRandomBot)
    {
        /*pmo = sPerformanceMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_EqSets");
        sLog.outDetail("Initializing second equipment set...");
        InitSecondEquipmentSet();
        if (pmo) pmo->finish();*/

        pmo = sPerformanceMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Inventory");
        sLog.outDetail("Initializing inventory...");
        InitInventory();
        if (pmo) pmo->finish();
    }

    if (isRandomBot)
    {
        pmo = sPerformanceMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Guilds & ArenaTeams");
        sLog.outDetail("Initializing guilds & ArenaTeams");
        InitGuild();
#ifndef MANGOSBOT_ZERO
        if (bot->GetLevel() >= 70)
            InitArenaTeam();
#endif
        if (pmo) pmo->finish();
    }

	if (bot->GetLevel() >= 10) {
		pmo = sPerformanceMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Pet");
		sLog.outDetail("Initializing pet...");
		InitPet();
        InitPetSpells();
		if (pmo) pmo->finish();
	}

    if (isRandomBot)
    {
        if (incremental)
        {
            uint32 money = bot->GetMoney();
            bot->SetMoney(money + 1000 * urand(1, level * 5));
        }
        else
        {
            bot->SetMoney(10000 * urand(1, level * 5));
        }
    }

    if (isRandomBot)
    {
        pmo = sPerformanceMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_TaxiNodes");
        sLog.outDetail("Initializing taxi...");
        InitTaxiNodes();
        if (pmo) pmo->finish();
    }

    pmo = sPerformanceMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Save");
    sLog.outDetail("Saving to DB...");
    if (sRandomPlayerbotMgr.GetDatabaseDelay("CharacterDatabase") < 10 * IN_MILLISECONDS)
        bot->SaveToDB();
    sLog.outDetail("Done.");
    if (pmo) pmo->finish();
}

void PlayerbotFactory::Refresh()
{
    //Prepare();
    if (!ai->HasCheat(BotCheatMask::item))
        return;

    InitAmmo();
    InitFood();
    InitPotions();
    InitReagents();
    AddConsumables();
    if(sRandomPlayerbotMgr.GetDatabaseDelay("CharacterDatabase") < 10 * IN_MILLISECONDS)
        bot->SaveToDB();
}

void PlayerbotFactory::AddConsumables()
{
   switch (bot->getClass())
   {
      case CLASS_PRIEST:
      case CLASS_MAGE:
      case CLASS_WARLOCK:
      {
         if (level >= 5 && level < 20) {
            StoreItem(CONSUM_ID_MINOR_WIZARD_OIL, 5);
            }
         if (level >= 20 && level < 40) {
            StoreItem(CONSUM_ID_MINOR_MANA_OIL, 5);
            StoreItem(CONSUM_ID_MINOR_WIZARD_OIL, 5);
         }
         if (level >= 40 && level < 45) {
             StoreItem(CONSUM_ID_MINOR_MANA_OIL, 5);
             StoreItem(CONSUM_ID_WIZARD_OIL, 5);
         }
#ifdef MANGOSBOT_ZERO
         if (level >= 45) {
             StoreItem(CONSUM_ID_BRILLIANT_MANA_OIL, 5);
             StoreItem(CONSUM_ID_BRILLIANT_WIZARD_OIL, 5);
         }
#else
         if (level >= 45 && level < 52) {
            StoreItem(CONSUM_ID_BRILLIANT_MANA_OIL, 5);
            StoreItem(CONSUM_ID_BRILLIANT_WIZARD_OIL, 5);
         }
         if (level >= 52 && level < 58) {
            StoreItem(CONSUM_ID_SUPERIOR_MANA_OIL, 5);
            StoreItem(CONSUM_ID_BRILLIANT_WIZARD_OIL, 5);
         }
         if (level >= 58 && level < 72) {
           StoreItem(CONSUM_ID_SUPERIOR_MANA_OIL, 5);
           StoreItem(CONSUM_ID_SUPERIOR_WIZARD_OIL, 5);
      }
#endif
   }
      break;
      case CLASS_PALADIN:
      case CLASS_WARRIOR:
      case CLASS_HUNTER:
       {
         if (level >= 1 && level < 5) {
            StoreItem(CONSUM_ID_ROUGH_SHARPENING_STONE, 5);
            StoreItem(CONSUM_ID_ROUGH_WEIGHTSTONE, 5);
        }
         if (level >= 5 && level < 15) {
            StoreItem(CONSUM_ID_COARSE_WEIGHTSTONE, 5);
            StoreItem(CONSUM_ID_COARSE_SHARPENING_STONE, 5);
         }
         if (level >= 15 && level < 25) {
            StoreItem(CONSUM_ID_HEAVY_WEIGHTSTONE, 5);
            StoreItem(CONSUM_ID_HEAVY_SHARPENING_STONE, 5);
         }
         if (level >= 25 && level < 35) {
            StoreItem(CONSUM_ID_SOL_SHARPENING_STONE, 5);
            StoreItem(CONSUM_ID_SOLID_WEIGHTSTONE, 5);
         }
#ifdef MANGOSBOT_ZERO
         if (level >= 35) {
             StoreItem(CONSUM_ID_DENSE_WEIGHTSTONE, 5);
             StoreItem(CONSUM_ID_DENSE_SHARPENING_STONE, 5);
         }
#else
         if (level >= 35 && level < 50) {
            StoreItem(CONSUM_ID_DENSE_WEIGHTSTONE, 5);
            StoreItem(CONSUM_ID_DENSE_SHARPENING_STONE, 5);
         }
         if (level >= 50 && level < 60) {
            StoreItem(CONSUM_ID_FEL_SHARPENING_STONE, 5);
            StoreItem(CONSUM_ID_FEL_WEIGHTSTONE, 5);
         }
         if (level >= 60) {
            StoreItem(CONSUM_ID_ADAMANTITE_WEIGHTSTONE, 5);
            StoreItem(CONSUM_ID_ADAMANTITE_SHARPENING_STONE, 5);
         }
#endif
   }
       break;
       case CLASS_ROGUE:
      {
         if (level >= 20 && level < 28) {
            StoreItem(CONSUM_ID_INSTANT_POISON, 5);
            StoreItem(CONSUM_ID_CRIPPLING_POISON, 5);
         }
         if (level >= 28 && level < 30) {
            StoreItem(CONSUM_ID_INSTANT_POISON_II, 5);
            StoreItem(CONSUM_ID_CRIPPLING_POISON, 5);
            StoreItem(CONSUM_ID_MIND_POISON, 5);
         }
         if (level >= 30 && level < 36) {
            StoreItem(CONSUM_ID_DEADLY_POISON, 5);
            StoreItem(CONSUM_ID_INSTANT_POISON_II, 5);
            StoreItem(CONSUM_ID_CRIPPLING_POISON, 5);
            StoreItem(CONSUM_ID_MIND_POISON, 5);
         }
         if (level >= 36 && level < 38) {
             StoreItem(CONSUM_ID_DEADLY_POISON, 5);
             StoreItem(CONSUM_ID_INSTANT_POISON_III, 5);
             StoreItem(CONSUM_ID_CRIPPLING_POISON, 5);
             StoreItem(CONSUM_ID_MIND_POISON, 5);
         }
         if (level >= 38 && level < 44) {
             StoreItem(CONSUM_ID_DEADLY_POISON_II, 5);
             StoreItem(CONSUM_ID_INSTANT_POISON_III, 5);
             StoreItem(CONSUM_ID_CRIPPLING_POISON, 5);
             StoreItem(CONSUM_ID_MIND_POISON_II, 5);
         }
         if (level >= 44 && level < 46) {
             StoreItem(CONSUM_ID_DEADLY_POISON_II, 5);
            StoreItem(CONSUM_ID_INSTANT_POISON_IV, 5);
            StoreItem(CONSUM_ID_CRIPPLING_POISON, 5);
            StoreItem(CONSUM_ID_MIND_POISON_II, 5);
         }
         if (level >= 46 && level < 52) {
             StoreItem(CONSUM_ID_DEADLY_POISON_III, 5);
             StoreItem(CONSUM_ID_INSTANT_POISON_IV, 5);
             StoreItem(CONSUM_ID_CRIPPLING_POISON, 5);
             StoreItem(CONSUM_ID_MIND_POISON_II, 5);
         }
         if (level >= 52 && level < 54) {
             StoreItem(CONSUM_ID_DEADLY_POISON_III, 5);
            StoreItem(CONSUM_ID_INSTANT_POISON_V, 5);
            StoreItem(CONSUM_ID_CRIPPLING_POISON_II, 5);
            StoreItem(CONSUM_ID_MIND_POISON_III, 5);
         }
         if (level >= 54 && level < 60) {
             StoreItem(CONSUM_ID_DEADLY_POISON_IV, 5);
             StoreItem(CONSUM_ID_INSTANT_POISON_V, 5);
             StoreItem(CONSUM_ID_CRIPPLING_POISON_II, 5);
             StoreItem(CONSUM_ID_MIND_POISON_III, 5);
         }
         if (level >= 60 && level < 62) {
            StoreItem(CONSUM_ID_DEADLY_POISON_V, 5);
            StoreItem(CONSUM_ID_INSTANT_POISON_VI, 5);
            StoreItem(CONSUM_ID_CRIPPLING_POISON_II, 5);
            StoreItem(CONSUM_ID_MIND_POISON_III, 5);
         }
         if (level >= 62 && level < 68) {
            StoreItem(CONSUM_ID_DEADLY_POISON_VI, 5);
            StoreItem(CONSUM_ID_INSTANT_POISON_VI, 5);
         }
         if (level >= 68 && level < 70) {
             StoreItem(CONSUM_ID_DEADLY_POISON_VI, 5);
            StoreItem(CONSUM_ID_INSTANT_POISON_VII, 5);
         }
         if (level >= 70 && level < 73) {
             StoreItem(CONSUM_ID_DEADLY_POISON_VII, 5);
             StoreItem(CONSUM_ID_INSTANT_POISON_VII, 5);
         }
         if (level >= 73 && level < 76) {
             StoreItem(CONSUM_ID_DEADLY_POISON_VII, 5);
             StoreItem(CONSUM_ID_INSTANT_POISON_VIII, 5);
         }
         if (level >= 76 && level < 79) {
             StoreItem(CONSUM_ID_DEADLY_POISON_VIII, 5);
             StoreItem(CONSUM_ID_INSTANT_POISON_VIII, 5);
         }
         if (level >= 79 && level < 80) {
             StoreItem(CONSUM_ID_DEADLY_POISON_VIII, 5);
             StoreItem(CONSUM_ID_INSTANT_POISON_IX, 5);
         }
         if (level == 80) {
             StoreItem(CONSUM_ID_DEADLY_POISON_IX, 5);
             StoreItem(CONSUM_ID_INSTANT_POISON_IX, 5);
         }
         break;
      }
   }
}

void PlayerbotFactory::InitPet()
{
    // Randomize a new pet (only for hunters)
    if (bot->getClass() != CLASS_HUNTER)
        return;

    Pet* pet = bot->GetPet();
    if (!pet)
    {
        Map* map = bot->GetMap();
        if (!map)
            return;

		vector<uint32> ids;
        for (uint32 id = 0; id < sCreatureStorage.GetMaxEntry(); ++id)
        {
            CreatureInfo const* co = sCreatureStorage.LookupEntry<CreatureInfo>(id);
			if (!co)
				continue;

#ifdef MANGOSBOT_TWO
            if (!co->isTameable(bot->CanTameExoticPets()))
#else
            if (!co->isTameable())
#endif
                continue;

            if ((int)co->MinLevel > (int)bot->GetLevel())
                continue;

			ids.push_back(id);
		}

        if (ids.empty())
        {
            sLog.outError("No pets available for bot %s (%d level)", bot->GetName(), bot->GetLevel());
            return;
        }

		for (int i = 0; i < 100; i++)
		{
			int index = urand(0, ids.size() - 1);
            CreatureInfo const* co = sCreatureStorage.LookupEntry<CreatureInfo>(ids[index]);
            if (!co)
                continue;

            uint32 guid = map->GenerateLocalLowGuid(HIGHGUID_PET);
#ifdef MANGOSBOT_TWO
            CreatureCreatePos pos(map, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetOrientation(), bot->GetPhaseMask());
#else
            CreatureCreatePos pos(map, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetOrientation());
#endif
            uint32 pet_number = sObjectMgr.GeneratePetNumber();
            pet = new Pet(HUNTER_PET);
            if (!pet->Create(guid, pos, co, pet_number))
            {
                delete pet;
                pet = NULL;
                continue;
            }

            pet->SetOwnerGuid(bot->GetObjectGuid());
            pet->SetGuidValue(UNIT_FIELD_CREATEDBY, bot->GetObjectGuid());
            pet->setFaction(bot->GetFaction());
            pet->SetLevel(bot->GetLevel());
            pet->InitStatsForLevel(bot->GetLevel());
#ifndef MANGOSBOT_TWO
            pet->SetLoyaltyLevel(BEST_FRIEND);
#endif
            pet->SetPower(POWER_HAPPINESS, HAPPINESS_LEVEL_SIZE * 2);
            pet->GetCharmInfo()->SetPetNumber(pet->GetObjectGuid().GetEntry(), true);
            pet->GetMap()->Add((Creature*)pet);
            pet->AIM_Initialize();
            pet->InitPetCreateSpells();
            pet->LearnPetPassives();
            pet->CastPetAuras(true);
            pet->CastOwnerTalentAuras();
            pet->UpdateAllStats();
            bot->SetPet(pet);
            bot->SetPetGuid(pet->GetObjectGuid());

            sLog.outDebug(  "Bot %s: assign pet %d (%d level)", bot->GetName(), co->Entry, bot->GetLevel());
            pet->SavePetToDB(PET_SAVE_AS_CURRENT, bot);
            bot->PetSpellInitialize();
            break;
        }
    }

    pet = bot->GetPet();
    if (pet)
    {
        pet->InitStatsForLevel(bot->GetLevel());
        pet->SetLevel(bot->GetLevel());
#ifndef MANGOSBOT_TWO
        pet->SetLoyaltyLevel(BEST_FRIEND);
#endif
        pet->SetPower(POWER_HAPPINESS, HAPPINESS_LEVEL_SIZE * 2);
        pet->SetHealth(pet->GetMaxHealth());
        pet->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
        pet->AI()->SetReactState(REACT_DEFENSIVE);
    }
    else
    {
        sLog.outError("Cannot create pet for bot %s", bot->GetName());
        return;
    }

    for (PetSpellMap::const_iterator itr = pet->m_spells.begin(); itr != pet->m_spells.end(); ++itr)
    {
        if(itr->second.state == PETSPELL_REMOVED)
            continue;

        uint32 spellId = itr->first;
        if(IsPassiveSpell(spellId))
            continue;

        pet->ToggleAutocast(spellId, true);
    }

    // Force dismiss pet to fix missing flags
    pet->ForcedDespawn();
}

void PlayerbotFactory::InitPetSpells()
{
    Map* map = bot->GetMap();
    if (!map)
        return;

    Pet* pet = bot->GetPet();
    if (pet)
    {
        if (bot->getClass() == CLASS_HUNTER)
        {
            // TO DO
            // ...
        }
// Warlock pets should auto learn spells in WOTLK
#ifndef MANGOSBOT_TWO
        else if (bot->getClass() == CLASS_WARLOCK)
        {
            constexpr uint32 PET_IMP = 416;
            constexpr uint32 PET_FELHUNTER = 417;
            constexpr uint32 PET_VOIDWALKER = 1860;
            constexpr uint32 PET_SUCCUBUS = 1863;
            constexpr uint32 PET_FELGUARD = 17252;

            //      pet type                    pet level  pet spell id
            std::map<uint32, std::vector<std::pair<uint32, uint32>>> spellList;

            // Imp spells
            {
                // Blood Pact
                spellList[PET_IMP].push_back(std::pair(4, 6307));
                spellList[PET_IMP].push_back(std::pair(14, 7804));
                spellList[PET_IMP].push_back(std::pair(26, 7805));
                spellList[PET_IMP].push_back(std::pair(38, 11766));
                spellList[PET_IMP].push_back(std::pair(50, 11767));
                spellList[PET_IMP].push_back(std::pair(62, 27268));
                spellList[PET_IMP].push_back(std::pair(74, 47982));

                // Fire Shield
                spellList[PET_IMP].push_back(std::pair(14, 2947));
                spellList[PET_IMP].push_back(std::pair(24, 8316));
                spellList[PET_IMP].push_back(std::pair(34, 8317));
                spellList[PET_IMP].push_back(std::pair(44, 11770));
                spellList[PET_IMP].push_back(std::pair(54, 11771));
                spellList[PET_IMP].push_back(std::pair(64, 27269));
                spellList[PET_IMP].push_back(std::pair(76, 47983));

                // Firebolt
                spellList[PET_IMP].push_back(std::pair(1, 3110));
                spellList[PET_IMP].push_back(std::pair(8, 7799));
                spellList[PET_IMP].push_back(std::pair(18, 7800));
                spellList[PET_IMP].push_back(std::pair(28, 7801));
                spellList[PET_IMP].push_back(std::pair(38, 7802));
                spellList[PET_IMP].push_back(std::pair(48, 11762));
                spellList[PET_IMP].push_back(std::pair(58, 11763));
                spellList[PET_IMP].push_back(std::pair(68, 27267));
                spellList[PET_IMP].push_back(std::pair(78, 47964));

                // Phase Shift
                spellList[PET_IMP].push_back(std::pair(12, 4511));
            }

            // Felhunter spells
            {
                // Devour Magic
                spellList[PET_FELHUNTER].push_back(std::pair(30, 19505));
                spellList[PET_FELHUNTER].push_back(std::pair(38, 19731));
                spellList[PET_FELHUNTER].push_back(std::pair(46, 19734));
                spellList[PET_FELHUNTER].push_back(std::pair(54, 19736));
                spellList[PET_FELHUNTER].push_back(std::pair(62, 27276));
                spellList[PET_FELHUNTER].push_back(std::pair(70, 27277));
                spellList[PET_FELHUNTER].push_back(std::pair(77, 48011));

                // Paranoia
                spellList[PET_FELHUNTER].push_back(std::pair(42, 19480));

                // Spell Lock
                spellList[PET_FELHUNTER].push_back(std::pair(36, 19244));
                spellList[PET_FELHUNTER].push_back(std::pair(52, 19647));

                // Tainted Blood
                spellList[PET_FELHUNTER].push_back(std::pair(32, 19478));
                spellList[PET_FELHUNTER].push_back(std::pair(40, 19655));
                spellList[PET_FELHUNTER].push_back(std::pair(48, 19656));
                spellList[PET_FELHUNTER].push_back(std::pair(56, 19660));
                spellList[PET_FELHUNTER].push_back(std::pair(64, 27280));
            }

            // Voidwalker spells
            {
                // Consume Shadows
                spellList[PET_VOIDWALKER].push_back(std::pair(18, 17767));
                spellList[PET_VOIDWALKER].push_back(std::pair(26, 17850));
                spellList[PET_VOIDWALKER].push_back(std::pair(34, 17851));
                spellList[PET_VOIDWALKER].push_back(std::pair(42, 17852));
                spellList[PET_VOIDWALKER].push_back(std::pair(50, 17853));
                spellList[PET_VOIDWALKER].push_back(std::pair(58, 17854));
                spellList[PET_VOIDWALKER].push_back(std::pair(66, 27272));
                spellList[PET_VOIDWALKER].push_back(std::pair(73, 47987));
                spellList[PET_VOIDWALKER].push_back(std::pair(78, 47988));

                // Sacrifice
                spellList[PET_VOIDWALKER].push_back(std::pair(16, 7812));
                spellList[PET_VOIDWALKER].push_back(std::pair(24, 19438));
                spellList[PET_VOIDWALKER].push_back(std::pair(32, 19440));
                spellList[PET_VOIDWALKER].push_back(std::pair(40, 19441));
                spellList[PET_VOIDWALKER].push_back(std::pair(48, 19442));
                spellList[PET_VOIDWALKER].push_back(std::pair(56, 19443));
                spellList[PET_VOIDWALKER].push_back(std::pair(64, 27273));
                spellList[PET_VOIDWALKER].push_back(std::pair(72, 47985));
                spellList[PET_VOIDWALKER].push_back(std::pair(79, 47986));

                // Suffering
                spellList[PET_VOIDWALKER].push_back(std::pair(24, 17735));
                spellList[PET_VOIDWALKER].push_back(std::pair(36, 17750));
                spellList[PET_VOIDWALKER].push_back(std::pair(48, 17751));
                spellList[PET_VOIDWALKER].push_back(std::pair(60, 17752));
                spellList[PET_VOIDWALKER].push_back(std::pair(63, 27271));
                spellList[PET_VOIDWALKER].push_back(std::pair(69, 33701));
                spellList[PET_VOIDWALKER].push_back(std::pair(75, 47989));
                spellList[PET_VOIDWALKER].push_back(std::pair(80, 47990));

                // Torment
                spellList[PET_VOIDWALKER].push_back(std::pair(10, 3716));
                spellList[PET_VOIDWALKER].push_back(std::pair(20, 7809));
                spellList[PET_VOIDWALKER].push_back(std::pair(30, 7810));
                spellList[PET_VOIDWALKER].push_back(std::pair(40, 7811));
                spellList[PET_VOIDWALKER].push_back(std::pair(50, 11774));
                spellList[PET_VOIDWALKER].push_back(std::pair(60, 11775));
                spellList[PET_VOIDWALKER].push_back(std::pair(70, 27270));
                spellList[PET_VOIDWALKER].push_back(std::pair(80, 47984));
            }

            // Succubus spells
            {
                // Lash of Pain
                spellList[PET_SUCCUBUS].push_back(std::pair(20, 7814));
                spellList[PET_SUCCUBUS].push_back(std::pair(28, 7815));
                spellList[PET_SUCCUBUS].push_back(std::pair(36, 7816));
                spellList[PET_SUCCUBUS].push_back(std::pair(44, 11778));
                spellList[PET_SUCCUBUS].push_back(std::pair(52, 11779));
                spellList[PET_SUCCUBUS].push_back(std::pair(60, 11780));
                spellList[PET_SUCCUBUS].push_back(std::pair(68, 27274));
                spellList[PET_SUCCUBUS].push_back(std::pair(74, 47991));
                spellList[PET_SUCCUBUS].push_back(std::pair(80, 47992));

                // Lesser Invisibility
                spellList[PET_SUCCUBUS].push_back(std::pair(32, 7870));

                // Seduction
                spellList[PET_SUCCUBUS].push_back(std::pair(26, 6358));

                // Soothing Kiss
                spellList[PET_SUCCUBUS].push_back(std::pair(22, 6360));
                spellList[PET_SUCCUBUS].push_back(std::pair(34, 7813));
                spellList[PET_SUCCUBUS].push_back(std::pair(46, 11784));
                spellList[PET_SUCCUBUS].push_back(std::pair(58, 11785));
                spellList[PET_SUCCUBUS].push_back(std::pair(70, 27275));
            }

            // Felguard spells
            {
                // Anguish
                spellList[PET_FELGUARD].push_back(std::pair(50, 33698));
                spellList[PET_FELGUARD].push_back(std::pair(60, 33699));
                spellList[PET_FELGUARD].push_back(std::pair(69, 33700));
                spellList[PET_FELGUARD].push_back(std::pair(78, 47993));

                // Avoidance
                spellList[PET_FELGUARD].push_back(std::pair(60, 32233));

                // Cleave
                spellList[PET_FELGUARD].push_back(std::pair(50, 30213));
                spellList[PET_FELGUARD].push_back(std::pair(60, 30219));
                spellList[PET_FELGUARD].push_back(std::pair(68, 30223));
                spellList[PET_FELGUARD].push_back(std::pair(76, 47994));

                // Demonic Frenzy
                spellList[PET_FELGUARD].push_back(std::pair(56, 32850));

                // Intercept
                spellList[PET_FELGUARD].push_back(std::pair(52, 30151));
                spellList[PET_FELGUARD].push_back(std::pair(61, 30194));
                spellList[PET_FELGUARD].push_back(std::pair(69, 30198));
                spellList[PET_FELGUARD].push_back(std::pair(79, 47996));
            }

            // Learn the appropriate spells by level and type
            const auto& petSpellListItr = spellList.find(pet->GetEntry());
            if (petSpellListItr != spellList.end())
            {
                const auto& petSpellList = petSpellListItr->second;
                for (const auto& pair : petSpellListItr->second)
                {
                    const uint32& levelRequired = pair.first;
                    const uint32& spellID = pair.second;

                    if (pet->GetLevel() >= levelRequired)
                    {
                        pet->learnSpell(spellID);
                    }
                }
            }
        }
#endif
    }
}

void PlayerbotFactory::ClearSkills()
{
    for (int i = 0; i < sizeof(tradeSkills) / sizeof(uint32); ++i)
    {
        bot->SetSkill(tradeSkills[i], 0, 0, 0);
    }
    bot->SetUInt32Value(PLAYER_SKILL_INDEX(0), 0);
    bot->SetUInt32Value(PLAYER_SKILL_INDEX(1), 0);
}

void PlayerbotFactory::ClearSpells()
{
#ifdef MANGOS
    list<uint32> spells;
    for(PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
    {
        uint32 spellId = itr->first;
		if (itr->second.state == PLAYERSPELL_REMOVED || itr->second.disabled || IsPassiveSpell(spellId))
			continue;

        spells.push_back(spellId);
    }

    for (list<uint32>::iterator i = spells.begin(); i != spells.end(); ++i)
    {
        bot->removeSpell(*i, false, false);
    }
#endif
#ifdef CMANGOS
    bot->resetSpells();
#endif
}

void PlayerbotFactory::ResetQuests()
{
    ObjectMgr::QuestMap const& questTemplates = sObjectMgr.GetQuestTemplates();
    for (ObjectMgr::QuestMap::const_iterator i = questTemplates.begin(); i != questTemplates.end(); ++i)
    {
        Quest const* quest = i->second;

        uint32 entry = quest->GetQuestId();

        // remove all quest entries for 'entry' from quest log
        for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
        {
            uint32 quest = bot->GetQuestSlotQuestId(slot);
            if (quest == entry)
            {
                bot->SetQuestSlot(slot, 0);
            }
        }

        // reset rewarded for restart repeatable quest
        bot->getQuestStatusMap().erase(entry);
        //bot->getQuestStatusMap()[entry].m_rewarded = false;
        //bot->getQuestStatusMap()[entry].m_status = QUEST_STATUS_NONE;
    }
    //bot->UpdateForQuestWorldObjects();
    CharacterDatabase.PExecute("DELETE FROM character_queststatus WHERE guid = '%u'", bot->GetGUIDLow());
}

void PlayerbotFactory::InitReputations()
{
    // list of factions
    list<uint32> factions;

    // neutral
    if (level >= 60)
    {
        factions.push_back(910); // nozdormu
        factions.push_back(749); // hydraxian waterlords
        factions.push_back(529); // argent dawn
    }
    
    // pvp factions
    if (level >= 60)
    {
        if (bot->GetTeam() == ALLIANCE)
        {
            factions.push_back(890); // Silverwing Sentinels
            factions.push_back(730); // Stormpike Guard
            factions.push_back(509); // The League of Arathor
        }
        else
        {
            factions.push_back(729); // Frostwolf Clan
            factions.push_back(510); // The Defilers
            factions.push_back(889); // Warsong Outriders
        }
    }

#ifndef MANGOSBOT_ZERO
    // TBC factions
    if (level >= 60)
    {
        factions.push_back(942);  // cenarion expedition
        factions.push_back(935);  // sha'tar
        factions.push_back(1011); // lower city
        factions.push_back(989);  // keepers of time
        factions.push_back(967);  // violet eye
        factions.push_back(1015); // netherwing
        factions.push_back(1077); // shattered sun
        factions.push_back(1012); // ashtongue
        factions.push_back(970);  // sporegarr
        factions.push_back(933);  // consortium
        factions.push_back(1031); // sha'tari skyguard
        factions.push_back(933);

        if (bot->GetTeam() == ALLIANCE)
        {
            factions.push_back(946); // honor hold
            factions.push_back(978); // kurenai
        }
        else
        {
            factions.push_back(947); // thrallmar
            factions.push_back(941); // mag'har
            factions.push_back(922); // tranquillen
        }
    }
#endif

    for (auto faction : factions)
    {
#ifdef MANGOSBOT_ONE
        FactionEntry const* factionEntry = sFactionStore.LookupEntry<FactionEntry>(faction);
#else
        FactionEntry const* factionEntry = sFactionStore.LookupEntry(faction);
#endif

        if (!factionEntry || !factionEntry->HasReputation())
            continue;

        bot->GetReputationMgr().SetReputation(factionEntry, 42000);
    }
}

void PlayerbotFactory::InitSpells()
{
    for (int i = 0; i < 15; i++)
        InitAvailableSpells();
}

void PlayerbotFactory::InitTalentsTree(bool incremental)
{
    uint32 specNo = sRandomPlayerbotMgr.GetValue(bot->GetGUIDLow(), "specNo");
    if (incremental && specNo)
	{
        specNo -= 1;
	}
    else
    {
        uint32 point = urand(0, 100);
        uint8 cls = bot->getClass();
        uint32 p1 = sPlayerbotAIConfig.specProbability[cls][0];
        uint32 p2 = p1 + sPlayerbotAIConfig.specProbability[cls][1];

        specNo = (point < p1 ? 0 : (point < p2 ? 1 : 2));
        sRandomPlayerbotMgr.SetValue(bot, "specNo", specNo + 1);
    }

    InitTalents(specNo);

    if (bot->GetFreeTalentPoints()) {
        InitTalents(2 - specNo);
    }
}

class DestroyItemsVisitor : public IterateItemsVisitor
{
public:
    DestroyItemsVisitor(Player* bot) : IterateItemsVisitor(), bot(bot) {}

    virtual bool Visit(Item* item)
    {
        uint32 id = item->GetProto()->ItemId;
        if (CanKeep(id))
        {
            keep.insert(id);
            return true;
        }

        bot->DestroyItem(item->GetBagSlot(), item->GetSlot(), true);
        return true;
    }

private:
    bool CanKeep(uint32 id)
    {
        if (keep.find(id) != keep.end())
            return false;

        if (sPlayerbotAIConfig.IsInRandomQuestItemList(id))
            return true;

        return false;
    }

private:
    Player* bot;
    set<uint32> keep;

};

bool PlayerbotFactory::CanEquipArmor(ItemPrototype const* proto)
{
    if (bot->HasSkill(SKILL_SHIELD) && proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
        return true;

    if (bot->HasSkill(SKILL_PLATE_MAIL))
    {
        if (proto->SubClass != ITEM_SUBCLASS_ARMOR_PLATE)
            return false;
    }
    else if (bot->HasSkill(SKILL_MAIL))
    {
        if (proto->SubClass != ITEM_SUBCLASS_ARMOR_MAIL)
            return false;
    }
    else if (bot->HasSkill(SKILL_LEATHER))
    {
        if (proto->SubClass != ITEM_SUBCLASS_ARMOR_LEATHER)
            return false;
    }

    if (proto->Quality <= ITEM_QUALITY_NORMAL)
        return true;

    for (uint8 slot = 0; slot < EQUIPMENT_SLOT_END; ++slot)
    {
       if (slot == EQUIPMENT_SLOT_TABARD || slot == EQUIPMENT_SLOT_BODY)
          continue;

    if (slot == EQUIPMENT_SLOT_OFFHAND && bot->getClass() == CLASS_ROGUE && proto->Class != ITEM_CLASS_WEAPON)
       continue;

    if (slot == EQUIPMENT_SLOT_OFFHAND && bot->getClass() == CLASS_PALADIN && proto->SubClass != ITEM_SUBCLASS_ARMOR_SHIELD)
       continue;
    }

    uint8 sp = 0, ap = 0, tank = 0;
    for (int j = 0; j < MAX_ITEM_PROTO_STATS; ++j)
    {
        // for ItemStatValue != 0
        if(!proto->ItemStat[j].ItemStatValue)
            continue;

        AddItemStats(proto->ItemStat[j].ItemStatType, sp, ap, tank);
    }

    return CheckItemStats(sp, ap, tank);
}

bool PlayerbotFactory::CheckItemStats(uint8 sp, uint8 ap, uint8 tank)
{
    switch (bot->getClass())
    {
    case CLASS_PRIEST:
    case CLASS_MAGE:
    case CLASS_WARLOCK:
        if (!sp || ap > sp || tank > sp)
            return false;
        break;
    case CLASS_PALADIN:
    case CLASS_WARRIOR:
        if ((!ap && !tank) || sp > ap || sp > tank)
            return false;
        break;
    case CLASS_HUNTER:
    case CLASS_ROGUE:
        if (!ap || sp > ap || sp > tank)
            return false;
        break;
#ifdef MANGOSBOT_TWO
    case CLASS_DEATH_KNIGHT:
        if ((!ap && !tank) || sp > ap || sp > tank)
            return false;
        break;
#endif
    }

    return sp || ap || tank;
}

void PlayerbotFactory::AddItemStats(uint32 mod, uint8 &sp, uint8 &ap, uint8 &tank)
{
    switch (mod)
    {
    case ITEM_MOD_HEALTH:
    case ITEM_MOD_STAMINA:
    case ITEM_MOD_MANA:
    case ITEM_MOD_INTELLECT:
    case ITEM_MOD_SPIRIT:
#ifndef MANGOSBOT_ZERO
    case ITEM_MOD_HIT_SPELL_RATING:
        case ITEM_MOD_HASTE_RATING:
        case ITEM_MOD_HASTE_RANGED_RATING:
        case ITEM_MOD_CRIT_RANGED_RATING:
        case ITEM_MOD_HIT_RANGED_RATING:
#endif
#ifdef MANGOSBOT_TWO
    case ITEM_MOD_SPELL_HEALING_DONE:
    case ITEM_MOD_SPELL_DAMAGE_DONE:
    case ITEM_MOD_MANA_REGENERATION:
    case ITEM_MOD_ARMOR_PENETRATION_RATING:
    case ITEM_MOD_SPELL_POWER:
    case ITEM_MOD_HEALTH_REGEN:
    case ITEM_MOD_SPELL_PENETRATION:
#endif
        sp++;
        break;
    }

    switch (mod)
    {
    case ITEM_MOD_AGILITY:
    case ITEM_MOD_STRENGTH:
    case ITEM_MOD_HEALTH:
    case ITEM_MOD_STAMINA:
#ifndef MANGOSBOT_ZERO
    case ITEM_MOD_DEFENSE_SKILL_RATING:
    case ITEM_MOD_DODGE_RATING:
    case ITEM_MOD_PARRY_RATING:
    case ITEM_MOD_BLOCK_RATING:
    case ITEM_MOD_HIT_TAKEN_MELEE_RATING:
    case ITEM_MOD_HIT_TAKEN_RANGED_RATING:
    case ITEM_MOD_HIT_TAKEN_SPELL_RATING:
    case ITEM_MOD_CRIT_TAKEN_MELEE_RATING:
    case ITEM_MOD_CRIT_TAKEN_RANGED_RATING:
    case ITEM_MOD_CRIT_TAKEN_SPELL_RATING:
    case ITEM_MOD_HIT_TAKEN_RATING:
    case ITEM_MOD_CRIT_TAKEN_RATING:
    case ITEM_MOD_RESILIENCE_RATING:
#endif
#ifdef MANGOSBOT_TWO
    case ITEM_MOD_BLOCK_VALUE:
#endif
        tank++;
        break;
    }

    switch (mod)
    {
    case ITEM_MOD_HEALTH:
    case ITEM_MOD_STAMINA:
    case ITEM_MOD_AGILITY:
    case ITEM_MOD_STRENGTH:
#ifndef MANGOSBOT_ZERO
    case ITEM_MOD_HIT_MELEE_RATING:
    case ITEM_MOD_HIT_RANGED_RATING:
    case ITEM_MOD_CRIT_MELEE_RATING:
    case ITEM_MOD_CRIT_RANGED_RATING:
    case ITEM_MOD_HASTE_MELEE_RATING:
    case ITEM_MOD_HASTE_RANGED_RATING:
    case ITEM_MOD_HIT_RATING:
    case ITEM_MOD_CRIT_RATING:
    case ITEM_MOD_HASTE_RATING:
    case ITEM_MOD_EXPERTISE_RATING:
#endif
#ifdef MANGOSBOT_TWO
    case ITEM_MOD_ATTACK_POWER:
    case ITEM_MOD_RANGED_ATTACK_POWER:
    case ITEM_MOD_FERAL_ATTACK_POWER:
#endif
        ap++;
        break;
    }
}

void PlayerbotFactory::AddItemSpellStats(uint32 smod, uint8& sp, uint8& ap, uint8& tank)
{
    switch (smod)
    {
    case SPELL_AURA_MOD_DAMAGE_DONE:
    case SPELL_AURA_MOD_HEALING_DONE:
    case SPELL_AURA_MOD_SPELL_CRIT_CHANCE:
    case SPELL_AURA_MOD_POWER_REGEN:
#ifndef MANGOSBOT_ZERO
    case SPELL_AURA_MOD_MANA_REGEN_FROM_STAT:
    case SPELL_AURA_HASTE_SPELLS:
#endif
        sp++;
        break;
    }

    switch (smod)
    {
#ifndef MANGOSBOT_ZERO
    case SPELL_AURA_MOD_EXPERTISE:
#endif
    case SPELL_AURA_MOD_ATTACK_POWER:
    case SPELL_AURA_MOD_CRIT_PERCENT:
    case SPELL_AURA_MOD_HIT_CHANCE:
    case SPELL_AURA_MOD_RANGED_ATTACK_POWER:
    case SPELL_AURA_EXTRA_ATTACKS:
    case SPELL_AURA_MOD_MELEE_HASTE:
    case SPELL_AURA_MOD_RANGED_HASTE:
        ap++;
        break;
    }

    switch (smod)
    {
    case SPELL_AURA_MOD_PARRY_PERCENT:
    case SPELL_AURA_MOD_DODGE_PERCENT:
    case SPELL_AURA_MOD_BLOCK_PERCENT:
    case SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN:
    case SPELL_AURA_MOD_BASE_RESISTANCE_PCT:
    case SPELL_AURA_MOD_BASE_RESISTANCE:
        //case SPELL_AURA_MOD_BLOCK_SKILL:
    case SPELL_AURA_MOD_SKILL:
    case SPELL_AURA_MOD_SHIELD_BLOCKVALUE:
    case SPELL_AURA_MOD_SHIELD_BLOCKVALUE_PCT:
        //case SPELL_AURA_MOD_HEALING_RECEIVED:
        tank++;
        break;
    }
}


bool PlayerbotFactory::CanEquipWeapon(ItemPrototype const* proto)
{
   int tab = AiFactory::GetPlayerSpecTab(bot);

   switch (bot->getClass())
   {
   case CLASS_PRIEST:
      if (proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF &&
         proto->SubClass != ITEM_SUBCLASS_WEAPON_WAND &&
         proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE)
         return false;
      break;
   case CLASS_MAGE:
     if (proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF &&
         proto->SubClass != ITEM_SUBCLASS_WEAPON_WAND)
         return false;
      break;
   case CLASS_WARLOCK:
      if (proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF &&
         proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER &&
         proto->SubClass != ITEM_SUBCLASS_WEAPON_WAND &&
         proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD)
         return false;
      break;
   case CLASS_WARRIOR:
      if (tab == 1) //fury
      {
         if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_FIST &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_THROWN)
            return false;
      }
      if ((tab == 0) && (bot->GetLevel() > 10))   //arms
      {
         if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2 &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_POLEARM &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_THROWN)
            return false;
      }
      else //prot +lowlvl
      {
         if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_THROWN)
            return false;
      }
      break;
   case CLASS_PALADIN:
         if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD)
         return false;
      break;
   case CLASS_SHAMAN:
      if (tab == 1) //enh
      {
         if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_FIST &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2 &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2)
            return false;
      }
      else //ele,resto
      {
         if (proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF)
            return false;
      }
      break;
   case CLASS_DRUID:
      if (tab == 1) //feral
      {
         if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF)
            return false;
      }
      else //ele,resto
      {
         if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF)
            return false;
      }
      break;
   case CLASS_HUNTER:
         if (proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2 &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_POLEARM &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW)
            return false;
      break;
   case CLASS_ROGUE:
      if (tab == 0) //assa
      {
         if (proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_THROWN)
            return false;
      }
      else
      {
         if (proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_FIST &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_THROWN)
            return false;
      }
      break;
#ifdef MANGOSBOT_TWO
   case CLASS_DEATH_KNIGHT:
       if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 &&
           proto->SubClass != ITEM_SUBCLASS_WEAPON_POLEARM &&
           proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 &&
           proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2)
           return false;
       break;
#endif
   }

   return true;
}

bool PlayerbotFactory::CanEquipItem(ItemPrototype const* proto, uint32 desiredQuality)
{
    if (proto->Duration & 0x80000000)
        return false;

    if (proto->Quality != desiredQuality)
        return false;

    if (proto->Bonding == BIND_QUEST_ITEM || proto->Bonding == BIND_WHEN_USE)
        return false;

    if (proto->Class == ITEM_CLASS_CONTAINER)
        return true;

    uint32 requiredLevel = proto->RequiredLevel;
    if (!requiredLevel)
    {
        requiredLevel = sRandomItemMgr.GetMinLevelFromCache(proto->ItemId);
    }
    if (!requiredLevel)
        return false;

    return true;
}

void PlayerbotFactory::InitEquipment(bool incremental, bool syncWithMaster)
{
    uint32 oldGS = ai->GetEquipGearScore(bot, false, false);
    uint32 masterGS = 0;
    if(syncWithMaster && ai->GetMaster())
    {
        masterGS = ai->GetEquipGearScore(ai->GetMaster(), false, false);
    }

    bool isRandomBot = sRandomPlayerbotMgr.IsRandomBot(bot) && bot->GetPlayerbotAI() && !bot->GetPlayerbotAI()->HasRealPlayerMaster() && !bot->GetPlayerbotAI()->IsInRealGuild();
    if (!incremental)
    {
        DestroyItemsVisitor visitor(bot);
        ai->InventoryIterateItems(&visitor, IterateItemsMask::ITERATE_ITEMS_IN_EQUIP);
    }

    uint32 specId = sRandomItemMgr.GetPlayerSpecId(bot);
    if (specId == 0)
        return;

    // choose type of weapon
    uint32 weaponType = 0;
    if (bot->GetLevel() > 40 && (bot->getClass() == CLASS_PRIEST || bot->getClass() == CLASS_MAGE || bot->getClass() == CLASS_WARLOCK || bot->getClass() == CLASS_SHAMAN || specId == 29 || specId == 31))
    {
        weaponType = sRandomPlayerbotMgr.GetValue(bot, "weaponType");
        if (!weaponType || !incremental)
        {
            weaponType = urand(0, 1) ? (uint32)INVTYPE_WEAPON : (uint32)INVTYPE_2HWEAPON;
            sRandomPlayerbotMgr.SetValue(bot, "weaponType", weaponType);
        }
    }

    // update only limited amount of slots with worst items
    //map<uint32, bool> upgradeSlots;
    //if (incremental)
    //{
    //    vector<uint32> emptySlots;
    //    vector<uint32> itemIds;
    //    map<uint32, uint32> itemSlots;
    //    uint32 maxSlots = urand(1, 4);
    //    for (uint8 slot = 0; slot < EQUIPMENT_SLOT_END; ++slot)
    //        upgradeSlots[slot] = false;

    //    for (uint8 slot = 0; slot < EQUIPMENT_SLOT_END; ++slot)
    //    {
    //        if (slot == EQUIPMENT_SLOT_TABARD/* && !bot->GetGuildId()*/ || slot == EQUIPMENT_SLOT_TRINKET1 || slot == EQUIPMENT_SLOT_TRINKET2)
    //            continue;

    //        Item* oldItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
    //        if (!oldItem)
    //        {
    //            emptySlots.push_back(slot);
    //            continue;
    //        }

    //        ItemPrototype const* proto = oldItem->GetProto();
    //        if (proto)
    //        {
    //            if (proto->ItemLevel >= sPlayerbotAIConfig.randomGearMaxLevel)
    //                continue;

    //            itemIds.push_back(proto->ItemId);
    //            itemSlots[proto->ItemId] = slot;
    //        }
    //    }

    //    std::sort(itemIds.begin(), itemIds.end(), [specId](int a, int b)
    //        {
    //            ItemPrototype const* proto1 = sObjectMgr.GetItemPrototype(a);
    //            ItemPrototype const* proto2 = sObjectMgr.GetItemPrototype(b);
    //            return proto1->Quality * proto1->ItemLevel <= proto2->Quality * proto2->ItemLevel;
    //        });

    //    uint32 counter = 0;
    //    for (auto emptySlot : emptySlots)
    //    {
    //        if (counter > maxSlots)
    //            break;

    //        upgradeSlots[emptySlot] = true;
    //        counter++;
    //    }
    //    for (auto itemId : itemIds)
    //    {
    //        if (counter > maxSlots)
    //            break;

    //        upgradeSlots[itemSlots[itemId]] = true;
    //        counter++;
    //    }
    //}

    // unavailable legendaries list
    vector<uint32> lockedItems;
    lockedItems.push_back(30311); // Warp Slicer
    lockedItems.push_back(30312); // Infinity Blade
    lockedItems.push_back(30313); // Staff of Disentagration
    lockedItems.push_back(30314); // Phaseshift Bulwark
    lockedItems.push_back(30316); // Devastation
    lockedItems.push_back(30317); // Cosmic Infuser
    lockedItems.push_back(30318); // Netherstrand Longbow
    lockedItems.push_back(18582); // Twin Blades of Azzinoth
    lockedItems.push_back(18583); // Right Blade
    lockedItems.push_back(18584); // Left Blade
    lockedItems.push_back(22736); // Andonisus, Reaper of Souls
    lockedItems.push_back(23051); // Glaive of the Defender
    lockedItems.push_back(13262); // Ashbringer
    lockedItems.push_back(17142); // Shard of the Defiler
    lockedItems.push_back(17782); // Talisman of Binding Shard
    lockedItems.push_back(12947); // Alex's Ring of Audacity

    for(uint8 slot = 0; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (slot == EQUIPMENT_SLOT_TABARD && bot->GetGuildId())
            continue;

        if (slot == EQUIPMENT_SLOT_END || slot == EQUIPMENT_SLOT_TABARD)
            continue;

        /*if (incremental && upgradeSlots.size() && upgradeSlots[slot] != true && !(slot == EQUIPMENT_SLOT_TRINKET1 || slot == EQUIPMENT_SLOT_TRINKET2))
            continue;*/

        uint32 searchLevel = level;
        uint32 quality = ITEM_QUALITY_POOR;
        uint32 maxItemLevel = sPlayerbotAIConfig.randomGearMaxLevel;
        bool progressiveGear = sPlayerbotAIConfig.randomGearProgression;
        if(syncWithMaster && ai->GetMaster())
        {
            maxItemLevel = masterGS + sPlayerbotAIConfig.randomGearMaxDiff;
            progressiveGear = false;
            if (bot->GetLevel() != searchLevel)
            {
                searchLevel = bot->GetLevel();
            }
        }
        else
        {
            if (progressiveGear)
            {
                if (!incremental)
                {
                    if (level < 10)
                        quality = ITEM_QUALITY_POOR;
                    else if (level < 20)
                        quality = urand(ITEM_QUALITY_NORMAL, ITEM_QUALITY_UNCOMMON);
                    else if (level < 40)
                        quality = urand(ITEM_QUALITY_UNCOMMON, ITEM_QUALITY_RARE);
                    else if (level < 60)
#ifdef MANGOSBOT_ZERO
                        quality = urand(ITEM_QUALITY_UNCOMMON, ITEM_QUALITY_RARE);
                    else if (level < 70)
                        quality = urand(ITEM_QUALITY_RARE, ITEM_QUALITY_EPIC);
#endif
#ifdef MANGOSBOT_ONE
                        quality = urand(ITEM_QUALITY_UNCOMMON, ITEM_QUALITY_RARE);
                    else if (level < 70)
                        quality = urand(ITEM_QUALITY_UNCOMMON, ITEM_QUALITY_RARE);
                    else if (level < 80)
                        quality = urand(ITEM_QUALITY_RARE, ITEM_QUALITY_EPIC);
#endif
#ifdef MANGOSBOT_TWO
                    quality = urand(ITEM_QUALITY_UNCOMMON, ITEM_QUALITY_RARE);
                    else if (level < 70)
                        quality = urand(ITEM_QUALITY_UNCOMMON, ITEM_QUALITY_RARE);
                    else if (level < 80)
                        quality = urand(ITEM_QUALITY_UNCOMMON, ITEM_QUALITY_RARE);
                    else
                        quality = urand(ITEM_QUALITY_RARE, ITEM_QUALITY_EPIC);
#endif
                }
                else
                {
                    if (level < 10)
                        quality = ITEM_QUALITY_POOR;
                    else if (level < 20)
                        quality = ITEM_QUALITY_NORMAL;
                    else if (level < 40)
                        quality = ITEM_QUALITY_UNCOMMON;
                    else if (level < 60)
#ifdef MANGOSBOT_ZERO
                        quality = ITEM_QUALITY_UNCOMMON;
                    else if (level < 70)
                        quality = ITEM_QUALITY_RARE;
#endif
#ifdef MANGOSBOT_ONE
                        quality = ITEM_QUALITY_UNCOMMON;
                    else if (level < 70)
                        quality = ITEM_QUALITY_UNCOMMON;
                    else if (level < 80)
                        quality = ITEM_QUALITY_RARE;
#endif
#ifdef MANGOSBOT_TWO
                        quality = ITEM_QUALITY_UNCOMMON;
                    else if (level < 70)
                        quality = ITEM_QUALITY_UNCOMMON;
                    else if (level < 80)
                        quality = ITEM_QUALITY_UNCOMMON;
                    else
                        quality = ITEM_QUALITY_RARE;
#endif
                }
            }
            if (progressiveGear && !incremental && urand(0, 100) < 100 * sPlayerbotAIConfig.randomGearLoweringChance && quality > ITEM_QUALITY_NORMAL) {
                quality--;
            }
        }

        bool found = false;
        uint32 attempts = 0;
        do
        {
            // pick random shirt
            if (slot == EQUIPMENT_SLOT_BODY || slot == EQUIPMENT_SLOT_TABARD)
            {
                vector<uint32> ids = sRandomItemMgr.Query(60, 1, 1, slot, 1);
                sLog.outDetail("Bot #%d %s:%d <%s>: %u possible items for slot %d", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName(), ids.size(), slot);

                if (!ids.empty()) ahbot::Shuffle(ids);

                for (uint32 index = 0; index < ids.size(); ++index)
                {
                    uint32 newItemId = ids[index];

                    // filter item level
                    ItemPrototype const* proto = sObjectMgr.GetItemPrototype(newItemId);
                    if (!proto)
                        continue;

                    if (proto->Quality > ITEM_QUALITY_UNCOMMON)
                        continue;

                    // skip unique-equippable items if already have one in inventory
                    if (proto->Flags & ITEM_FLAG_UNIQUE_EQUIPPABLE && bot->HasItemCount(proto->ItemId, 1))
                        continue;

                    if (proto->MaxCount && bot->HasItemCount(proto->ItemId, proto->MaxCount))
                        continue;

                    if (proto->ItemLevel > sPlayerbotAIConfig.randomGearMaxLevel)
                        continue;

                    Item* oldItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
                    ItemPrototype const* oldProto = oldItem ? oldItem->GetProto() : nullptr;

                    if (oldItem && oldProto->ItemId == newItemId)
                        continue;

                    uint16 eDest;
                    if (CanEquipUnseenItem(slot, eDest, newItemId))
                    {
                        if (oldItem)
                            bot->DestroyItem(oldItem->GetBagSlot(), oldItem->GetSlot(), true);

                        Item* pItem = bot->EquipNewItem(eDest, newItemId, true);
                        if (pItem)
                            found = true;
                    }
                    if (found)
                        break;
                }
            }
            else
            {
                vector<uint32> ids;
                for (uint32 q = quality; q < ITEM_QUALITY_ARTIFACT; ++q)
                {
                    uint32 currSearchLevel = searchLevel;
                    bool hasProperLevel = false;
                    while (!hasProperLevel && currSearchLevel > 0)
                    {
                        vector<uint32> newItems = sRandomItemMgr.Query(currSearchLevel, bot->getClass(), uint8(specId), slot, q);
                        if (newItems.size())
                            ids.insert(ids.begin(), newItems.begin(), newItems.end());

                        for (auto id : ids)
                        {
                            ItemPrototype const* proto = sObjectMgr.GetItemPrototype(id);
                            if(proto)
                            {
                                if (proto->ItemLevel > maxItemLevel)
                                    continue;

                                hasProperLevel = true;
                                break;
                            }
                        }

                        if (!hasProperLevel)
                        {
                            ids.clear();
                            currSearchLevel--;
                        }
                    }

                    // add one hand weapons for tanks
                    if ((specId == 3 || specId == 5) && slot == EQUIPMENT_SLOT_MAINHAND)
                    {
                        vector<uint32> oneHanded = sRandomItemMgr.Query(level, bot->getClass(), uint8(specId), EQUIPMENT_SLOT_OFFHAND, q);
                        if (oneHanded.size())
                            ids.insert(ids.begin(), oneHanded.begin(), oneHanded.end());
                    }

                    // add one hand weapons for casters
                    if ((specId == 4 || (bot->getClass() == CLASS_DRUID || bot->getClass() == CLASS_PRIEST || bot->getClass() == CLASS_MAGE || bot->getClass() == CLASS_WARLOCK || (specId == 20 || specId == 22))) && slot == EQUIPMENT_SLOT_MAINHAND)
                    {
                        vector<uint32> oneHanded = sRandomItemMgr.Query(level, bot->getClass(), uint8(specId), EQUIPMENT_SLOT_OFFHAND, q);
                        if (oneHanded.size())
                            ids.insert(ids.begin(), oneHanded.begin(), oneHanded.end());
                    }

                    // add weapons for dual wield
                    if (slot == EQUIPMENT_SLOT_MAINHAND && (bot->getClass() == CLASS_ROGUE || specId == 2 || specId == 21))
                    {
                        vector<uint32> oneHanded = sRandomItemMgr.Query(level, bot->getClass(), uint8(specId), EQUIPMENT_SLOT_OFFHAND, q);
                        if (oneHanded.size())
                            ids.insert(ids.begin(), oneHanded.begin(), oneHanded.end());
                    }
                }

                sLog.outDetail("Bot #%d %s:%d <%s>: %u possible items for slot %d", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName(), ids.size(), slot);

                if (incremental || !progressiveGear)
                {
                    // sort items based on stat value, ilvl or quality
                    std::sort(ids.begin(), ids.end(), [specId](int a, int b)
                        {
                            uint32 baseCompareA = sRandomItemMgr.GetStatWeight(a, specId) * 1000;
                            uint32 baseCompareB = sRandomItemMgr.GetStatWeight(b, specId) * 1000;
                            if (baseCompareA < baseCompareB)
                                return true;

                            ItemPrototype const* proto1 = sObjectMgr.GetItemPrototype(a);
                            ItemPrototype const* proto2 = sObjectMgr.GetItemPrototype(b);

                            baseCompareA += proto1->Quality * proto1->ItemLevel;
                            baseCompareB += proto2->Quality * proto2->ItemLevel;

                            return baseCompareA < baseCompareB;
                        });

                    if (!progressiveGear)
                        std::reverse(ids.begin(), ids.end());
                }
                else
                    if (!ids.empty()) ahbot::Shuffle(ids);

                for (uint32 index = 0; index < ids.size(); ++index)
                {
                    uint32 newItemId = ids[index];

                    // filter item level
                    ItemPrototype const* proto = sObjectMgr.GetItemPrototype(newItemId);
                    if (!proto)
                        continue;

                    if (std::find(lockedItems.begin(), lockedItems.end(), proto->ItemId) != lockedItems.end())
                        continue;

                    // skip not available items
                    if (proto->ExtraFlags & ITEM_EXTRA_NOT_OBTAINABLE)
                        continue;

                    // blacklist
                    if (std::find(sPlayerbotAIConfig.randomGearBlacklist.begin(), sPlayerbotAIConfig.randomGearBlacklist.end(), proto->ItemId) != sPlayerbotAIConfig.randomGearBlacklist.end())
                        continue;

                    // skip unique-equippable items if already have one in inventory
                    if (proto->Flags & ITEM_FLAG_UNIQUE_EQUIPPABLE && bot->HasItemCount(proto->ItemId, 1))
                        continue;

                    if (proto->MaxCount && bot->HasItemCount(proto->ItemId, proto->MaxCount))
                        continue;

                    if (proto->ItemLevel > maxItemLevel)
                        continue;

                    // do not use items that required level is too low compared to bot's level
                    uint32 reqLevel = sRandomItemMgr.GetMinLevelFromCache(newItemId);
                    if (reqLevel && proto->Quality < ITEM_QUALITY_LEGENDARY && abs((int)bot->GetLevel() - (int)reqLevel) > (int)sPlayerbotAIConfig.randomGearMaxDiff)
                        continue;

                    // filter tank weapons
                    if (slot == EQUIPMENT_SLOT_OFFHAND && (specId == 3 || specId == 5) && !(proto->Class == ITEM_CLASS_ARMOR && proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD))
                        continue;

                    if (slot == EQUIPMENT_SLOT_MAINHAND && proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
                        continue;

                    if (slot == EQUIPMENT_SLOT_MAINHAND && proto->InventoryType == INVTYPE_HOLDABLE)
                        continue;

                    // filter tank weapons
                    if (slot == EQUIPMENT_SLOT_MAINHAND && (specId == 3 || specId == 5) && !(proto->Class == ITEM_CLASS_WEAPON && proto->InventoryType != INVTYPE_HOLDABLE))
                        continue;

                    // make fury wear slow weapon as main hand
                    if (slot == EQUIPMENT_SLOT_MAINHAND && specId == 2 && proto->IsWeapon() && proto->Delay < 2000)
                        continue;

#ifdef MANGOSBOT_ONE
                    // tbc make enhancement shaman use weapons above 2.0 speed in both hands
                    if ((slot == EQUIPMENT_SLOT_MAINHAND || slot == EQUIPMENT_SLOT_OFFHAND) && bot->GetLevel() >= 60 && specId == 21 && proto->IsWeapon() && proto->InventoryType != INVTYPE_2HWEAPON && proto->Delay < 2000)
                        continue;
#endif
                    // classic enh shaman and retri paladin 60+ use weapon speed >= 3.0
                    if (slot == EQUIPMENT_SLOT_MAINHAND && (specId == 6 || specId == 21) && proto->IsWeapon() && bot->GetLevel() >= 60 && proto->InventoryType == INVTYPE_2HWEAPON && proto->Delay < 3000)
                        continue;

                    // filter caster weapons
                    if (weaponType)
                    {
                        if (slot == EQUIPMENT_SLOT_MAINHAND)
                        {
                            if (proto->Class == ITEM_CLASS_ARMOR && proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
                                continue;
                            if (weaponType == INVTYPE_2HWEAPON && proto->Class == ITEM_CLASS_WEAPON && proto->InventoryType != INVTYPE_2HWEAPON)
                                continue;
                            if (weaponType != INVTYPE_2HWEAPON && proto->Class == ITEM_CLASS_WEAPON && proto->InventoryType == INVTYPE_2HWEAPON)
                                continue;
                            if (proto->InventoryType == INVTYPE_HOLDABLE)
                                continue;
                        }
                        if (slot == EQUIPMENT_SLOT_OFFHAND)
                        {
                            if (weaponType != INVTYPE_2HWEAPON && (bot->getClass() == CLASS_PRIEST || bot->getClass() == CLASS_MAGE || bot->getClass() == CLASS_WARLOCK || (bot->getClass() == CLASS_DRUID && (specId == 29 || specId == 31))) && proto->InventoryType != INVTYPE_HOLDABLE)
                                continue;
                            if (weaponType == INVTYPE_2HWEAPON && (bot->getClass() == CLASS_PRIEST || bot->getClass() == CLASS_MAGE || bot->getClass() == CLASS_WARLOCK || (bot->getClass() == CLASS_DRUID && (specId == 29 || specId == 31))))
                                continue;

                            if (weaponType != INVTYPE_2HWEAPON && proto->Class == ITEM_CLASS_WEAPON && proto->InventoryType == INVTYPE_2HWEAPON)
                                continue;
                        }
                    }

                    Item* oldItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
                    ItemPrototype const* oldProto = oldItem ? oldItem->GetProto() : nullptr;
                    //uint32 oldStatValue = oldItem ? sRandomItemMgr.GetStatWeight(oldProto->ItemId, specId) : 0;
                    uint32 oldStatValue = oldItem ? sRandomItemMgr.GetLiveStatWeight(bot, oldProto->ItemId, specId) : 0;

                    if (oldItem && oldProto->ItemId == newItemId)
                        continue;

                    // chance to get legendary
                    if (proto->Quality == ITEM_QUALITY_LEGENDARY && urand(0, 100) > 20)
                        continue;

                    // chance to not replace legendary
                    if (incremental && oldItem && oldProto->Quality == ITEM_QUALITY_LEGENDARY && urand(0, 100) > uint32(100 * 0.5f))
                        continue;

                    uint32 newStatValue = sRandomItemMgr.GetLiveStatWeight(bot, newItemId, specId);
                    if (newStatValue <= 0)
                        continue;

                    // skip off hand if main hand is worse
#ifdef MANGOSBOT_ZERO
                    if (proto->IsWeapon() && slot == EQUIPMENT_SLOT_OFFHAND && (bot->getClass() == CLASS_ROGUE || specId == 2))
#endif
#ifdef MANGOSBOT_ONE
                    if (proto->IsWeapon() && slot == EQUIPMENT_SLOT_OFFHAND && (bot->getClass() == CLASS_ROGUE || specId == 2 || specId == 21))
#endif
#ifdef MANGOSBOT_TWO
                    if (proto->IsWeapon() && slot == EQUIPMENT_SLOT_OFFHAND && (bot->getClass() == CLASS_ROGUE || specId == 2 || specId == 21 || bot->getClass() == CLASS_DEATH_KNIGHT))
#endif
                        {
                            bool betterValue = false;
                            bool betterDamage = false;
                            bool betterDps = false;
                            Item* mhItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                            if (mhItem && mhItem->GetProto())
                            {
                                ItemPrototype const* mhProto = mhItem->GetProto();
                                uint32 mhStatValue = sRandomItemMgr.GetLiveStatWeight(bot, mhProto->ItemId, specId);
                                if (newStatValue > mhStatValue)
                                    betterValue = true;

                                uint32 mhDps = 0;
                                uint32 ohDps = 0;
                                uint32 mhDamage = 0;
                                uint32 ohDamage = 0;
                                for (int i = 0; i < MAX_ITEM_PROTO_DAMAGES; i++)
                                {
                                    if (mhProto->Damage[i].DamageMax == 0)
                                        break;

                                    mhDamage = mhProto->Damage[i].DamageMax;

                                    mhDps = (mhProto->Damage[i].DamageMin + mhProto->Damage[i].DamageMax) / (float)(mhProto->Delay / 1000.0f) / 2;
                                }
                                for (int i = 0; i < MAX_ITEM_PROTO_DAMAGES; i++)
                                {
                                    if (proto->Damage[i].DamageMax == 0)
                                        break;

                                    ohDamage = proto->Damage[i].DamageMax;

                                    ohDps = (proto->Damage[i].DamageMin + proto->Damage[i].DamageMax) / (float)(proto->Delay / 1000.0f) / 2;
                                }
                                if (ohDps > mhDps)
                                    betterDps = true;
                                if (ohDamage > mhDamage)
                                    betterDamage = true;
                            }
                            if (betterDps || (betterDamage && betterValue))
                                continue;
                        }

                    if (incremental && oldItem && oldStatValue >= newStatValue && oldStatValue > 1)
                        continue;

                    // replace grey items right away
                    if ((incremental || progressiveGear) && oldItem && oldProto->Quality < ITEM_QUALITY_NORMAL && proto->Quality < ITEM_QUALITY_NORMAL && level > 5)
                        continue;

                    uint16 eDest;
                    if (CanEquipUnseenItem(slot, eDest, newItemId))
                    {
                        if (oldItem)
                            bot->DestroyItem(oldItem->GetBagSlot(), oldItem->GetSlot(), true);

                        Item* pItem = bot->EquipNewItem(eDest, newItemId, true);
                        if (pItem)
                        {
                            pItem->SetOwnerGuid(bot->GetObjectGuid());
                            EnchantItem(pItem);
                            //AddGems(pItem);
                            found = true;
                        }
                    }
                    if (found)
                    {
                        if (incremental)
                        {
                            if (oldItem)
                                sLog.outDetail("Bot #%d %s:%d <%s>: Old Item: slot: %u, id: %u, value: %u (%s)", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName(), slot, oldProto->ItemId, oldStatValue, oldProto->Name1);
                            sLog.outDetail("Bot #%d %s:%d <%s>: New Item: slot: %u, id: %u, value: %u (%s)", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName(), slot, proto->ItemId, newStatValue, proto->Name1);
                        }
                        break;
                    }
                }
            }

            if (/*!incremental && */quality == ITEM_QUALITY_EPIC)
                quality--;

            attempts++;
        } while (!found && attempts < 2/* && (progressiveGear ? (quality != ITEM_QUALITY_ARTIFACT) : (quality != ITEM_QUALITY_POOR))*/);
        if (!found)
        {
            if (slot != EQUIPMENT_SLOT_TRINKET1 && slot != EQUIPMENT_SLOT_TRINKET2)
                sLog.outDetail("Bot #%d %s:%d <%s>: no items for slot %d, quality >= %u", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName(), slot, quality);
            continue;
        }
    }

    /*if (incremental && oldGS != newGS)
        sLog.outBasic("Bot #%d %s:%d <%s>: GS: %u -> %u", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName(), oldGS, newGS);*/

    // Update stats here so the bots will benefit from the new equipped items' stats
    bot->InitStatsForLevel(true);
    bot->UpdateAllStats();

    if(syncWithMaster && ai->GetMaster())
    {
        uint32 newGS = ai->GetEquipGearScore(bot, false, false);
        std::stringstream message;
        message << "Synced gear with master. Old GS: " << oldGS << " New GS: " << newGS << " Master GS: " << masterGS;
        ai->TellPlayerNoFacing(ai->GetMaster(), message.str());
    }
}

bool PlayerbotFactory::IsDesiredReplacement(uint32 itemId)
{
    if (!itemId)
        return true;

    ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
    if (!proto)
        return false;

    uint32 requiredLevel = proto->RequiredLevel;
    if (!requiredLevel)
    {
        requiredLevel = sRandomItemMgr.GetMinLevelFromCache(proto->ItemId);
    }
    if (!requiredLevel)
        return false;

    int delta = sPlayerbotAIConfig.randomGearMaxDiff + (80 - bot->GetLevel()) / 10;
    return (int)bot->GetLevel() - (int)requiredLevel > delta;
}

void PlayerbotFactory::InitSecondEquipmentSet()
{
    if (bot->getClass() == CLASS_MAGE || bot->getClass() == CLASS_WARLOCK || bot->getClass() == CLASS_PRIEST)
        return;

    map<uint32, vector<uint32> > items;

    uint32 desiredQuality = ITEM_QUALITY_NORMAL;
    if (level < 10)
        desiredQuality = urand(ITEM_QUALITY_POOR, ITEM_QUALITY_UNCOMMON);
    if (level < 20)
        desiredQuality = urand(ITEM_QUALITY_NORMAL, ITEM_QUALITY_UNCOMMON);
    else if (level < 40)
        desiredQuality = urand(ITEM_QUALITY_UNCOMMON, ITEM_QUALITY_RARE);
    else if (level < 60)
#ifdef MANGOSBOT_ZERO
        desiredQuality = urand(ITEM_QUALITY_UNCOMMON, ITEM_QUALITY_RARE);
    else if (level < 70)
        desiredQuality = urand(ITEM_QUALITY_RARE, ITEM_QUALITY_EPIC);
#else
        desiredQuality = urand(ITEM_QUALITY_UNCOMMON, ITEM_QUALITY_RARE);
    else if (level < 70)
        desiredQuality = urand(ITEM_QUALITY_RARE, ITEM_QUALITY_EPIC);
#endif
    else if (level < 80)
        desiredQuality = urand(ITEM_QUALITY_UNCOMMON, ITEM_QUALITY_RARE);
    else
        desiredQuality = urand(ITEM_QUALITY_RARE, ITEM_QUALITY_EPIC);

    while (urand(0, 100) < 100 * sPlayerbotAIConfig.randomGearLoweringChance && desiredQuality > ITEM_QUALITY_NORMAL) {
        desiredQuality--;
    }

    do
    {
        for (uint32 itemId = 0; itemId < sItemStorage.GetMaxEntry(); ++itemId)
        {
            ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
            if (!proto)
                continue;

            // filter item level
            if (proto->ItemLevel > sPlayerbotAIConfig.randomGearMaxLevel)
                continue;

            // do not use items that required level is too low compared to bot's level
            uint32 reqLevel = sRandomItemMgr.GetMinLevelFromCache(itemId);
            if (reqLevel && proto->Quality < ITEM_QUALITY_LEGENDARY && abs((int)bot->GetLevel() - (int)reqLevel) > (int)sPlayerbotAIConfig.randomGearMaxDiff)
                continue;

            if (!CanEquipItem(proto, desiredQuality))
                continue;

            if (proto->Class == ITEM_CLASS_WEAPON)
            {
                if (!CanEquipWeapon(proto))
                    continue;

                Item* existingItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                if (existingItem)
                {
                    switch (existingItem->GetProto()->SubClass)
                    {
                    case ITEM_SUBCLASS_WEAPON_AXE:
                    case ITEM_SUBCLASS_WEAPON_DAGGER:
                    case ITEM_SUBCLASS_WEAPON_FIST:
                    case ITEM_SUBCLASS_WEAPON_MACE:
                    case ITEM_SUBCLASS_WEAPON_SWORD:
                        if (proto->SubClass == ITEM_SUBCLASS_WEAPON_AXE || proto->SubClass == ITEM_SUBCLASS_WEAPON_DAGGER ||
                            proto->SubClass == ITEM_SUBCLASS_WEAPON_FIST || proto->SubClass == ITEM_SUBCLASS_WEAPON_MACE ||
                            proto->SubClass == ITEM_SUBCLASS_WEAPON_SWORD)
                            continue;
                        break;
                    default:
                        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE && proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER &&
                            proto->SubClass != ITEM_SUBCLASS_WEAPON_FIST && proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
                            proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD)
                            continue;
                        break;
                    }
                }
            }
            else if (proto->Class == ITEM_CLASS_ARMOR && proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
            {
                Item* existingItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                if (existingItem && existingItem->GetProto()->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
                    continue;
            }
            else
                continue;

            items[proto->Class].push_back(itemId);
        }
    } while (items[ITEM_CLASS_ARMOR].empty() && items[ITEM_CLASS_WEAPON].empty() && desiredQuality++ != ITEM_QUALITY_ARTIFACT);

    int maxCount = urand(0, 5);
    int count = 0;
    for (map<uint32, vector<uint32> >::iterator i = items.begin(); i != items.end(); ++i)
    {
        if (count++ >= maxCount)
            break;

        vector<uint32>& ids = i->second;
        if (ids.empty())
        {
            sLog.outDebug(  "%s: no items to make second equipment set for slot %d", bot->GetName(), i->first);
            continue;
        }
        for (int attempts = 0; attempts < 15; attempts++)
        {
            uint32 index = urand(0, ids.size() - 1);
            uint32 newItemId = ids[index];
            Item* newItem = StoreItem(newItemId, 1);
            if (newItem)
            {
                count++;
                break;
            }
        }
    }
}

void PlayerbotFactory::InitBags()
{
    for (uint8 slot = INVENTORY_SLOT_BAG_START; slot < INVENTORY_SLOT_BAG_END; ++slot)
    {
        Bag* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (!pBag)
        {
#ifdef MANGOSBOT_ZERO
            bot->StoreNewItemInBestSlots(4500, 1); // add Traveler's Backpack if no bag in slot
#else

            bot->StoreNewItemInBestSlots(23162, 1);
#endif
        }
    }
}

void PlayerbotFactory::EnchantItem(Item* item)
{
    if (!item)
        return;

    if (bot->GetLevel() < sPlayerbotAIConfig.minEnchantingBotLevel)
        return;

    int tab = AiFactory::GetPlayerSpecTab(bot);
    uint32 tempId = uint32((uint32)bot->getClass() * (uint32)10);
    ApplyEnchantTemplate(tempId += (uint32)tab, item);
}

void PlayerbotFactory::AddGems(Item* item)
{
    if (!item)
        return;

#ifndef MANGOSBOT_ZERO
    if (ItemPrototype const* proto = item->GetProto())
    {
        uint32 gemsList[MAX_GEM_SOCKETS];
        ObjectGuid gem_guids[MAX_GEM_SOCKETS];
        bool gemCreated = false;
        bool hasSockets = false;

        for (int i = 0; i < MAX_GEM_SOCKETS; ++i)               // check for hack maybe
        {
            // tried to put gem in socket where no socket exists
            if (!proto->Socket[i].Color)
                continue;

            if (proto->Socket[i].Color)
                hasSockets = true;
        }
        if (!hasSockets)
            return;

        vector<uint32> gems = sRandomItemMgr.GetGemsList();
        if (gems.empty())
            return;

        ahbot::Shuffle(gems);

        for (uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT + MAX_GEM_SOCKETS; ++enchant_slot)
        {
            ObjectGuid gem_GUID;
            uint32 SocketColor = proto->Socket[enchant_slot - SOCK_ENCHANTMENT_SLOT].Color;
            uint32 gem_id = 0;
            switch (SocketColor) {
            case SOCKET_COLOR_META:
                gem_id = 25890;
                break;
            default:
            {
                for (vector<uint32>::const_iterator itr = gems.begin(); itr != gems.end(); itr++)
                {
                    if (ItemPrototype const* gemProto = sObjectMgr.GetItemPrototype(*itr))
                    {
                        if (GemPropertiesEntry const* gemProperty = sGemPropertiesStore.LookupEntry(gemProto->GemProperties))
                        {
                            if (SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(gemProperty->spellitemenchantement))
                            {
                                uint32 GemColor = gemProperty->color;

                                /*if (GemColor == 14)
                                    if ((bot->HasItemOrGemWithIdEquipped(gemProto->ItemId, 1)) || (bot->HasItemCount(gemProto->ItemId, 1)))
                                        continue;*/

                                if (gemProto->RequiredSkillRank > bot->GetSkillValue(SKILL_JEWELCRAFTING))
                                    continue;


                                /*if ((bot->GetLevel()) < (gemProto->ItemLevel - 10))
                                    continue;*/

                                uint8 sp = 0, ap = 0, tank = 0;
                                if (GemColor & SocketColor && GemColor == SocketColor)
                                {
                                    for (int i = 0; i < 3; ++i)
                                    {
                                        if (pEnchant->type[i] != ITEM_ENCHANTMENT_TYPE_STAT)
                                            continue;

                                        AddItemStats(pEnchant->spellid[i], sp, ap, tank);
                                    }
                                }

                                if (!CheckItemStats(sp, ap, tank))
                                    continue;

                                if (gemProto->RequiredSkillRank > bot->GetSkillValue(SKILL_JEWELCRAFTING))
                                    continue;

                                gem_id = gemProto->ItemId;
                                break;
                            }
                        }
                    }
                }
            }
            break;
            }
            if (gem_id > 0)
            {
                gemsList[enchant_slot - SOCK_ENCHANTMENT_SLOT] = gem_id;
                if (Item* gem = StoreItem(gem_id, 1))
                {
                    gem_GUID = gem->GetObjectGuid();
                    gem_guids[enchant_slot - SOCK_ENCHANTMENT_SLOT] = gem_GUID;
                    gemCreated = true;
                }
            }
        }
        if (gemCreated)
        {
            WorldPacket socket(CMSG_SOCKET_GEMS);
            socket << item->GetObjectGuid();
            for (int i = 0; i < MAX_GEM_SOCKETS; ++i)
            {
                socket << gem_guids[i];
            }
            bot->GetSession()->HandleSocketOpcode(socket);
        }
    }
#else
    return;
#endif
}

bool PlayerbotFactory::CanEquipUnseenItem(uint8 slot, uint16 &dest, uint32 item)
{
    dest = 0;
    Item* pItem = RandomPlayerbotMgr::CreateTempItem(item, 1, bot);
    
    if (pItem)
    {
        InventoryResult result = bot->CanEquipItem(slot, dest, pItem, true, false);
        pItem->RemoveFromUpdateQueueOf(bot);
        delete pItem;
        return result == EQUIP_ERR_OK;
    }

    return false;
}

void PlayerbotFactory::InitTradeSkills()
{
    uint16 firstSkill = sRandomPlayerbotMgr.GetValue(bot, "firstSkill");
    uint16 secondSkill = sRandomPlayerbotMgr.GetValue(bot, "secondSkill");
    if (!firstSkill || !secondSkill)
    {
        vector<uint32> firstSkills;
        vector<uint32> secondSkills;
        switch (bot->getClass())
        {
        case CLASS_WARRIOR:
        case CLASS_PALADIN:
#ifdef MANGOSBOT_TWO
        case CLASS_DEATH_KNIGHT:
#endif
            firstSkills.push_back(SKILL_BLACKSMITHING);
            secondSkills.push_back(SKILL_ENGINEERING);
            break;
        case CLASS_SHAMAN:
        case CLASS_DRUID:
        case CLASS_HUNTER:
        case CLASS_ROGUE:
            firstSkills.push_back(SKILL_SKINNING);
            firstSkills.push_back(SKILL_ENGINEERING);
            secondSkills.push_back(SKILL_LEATHERWORKING);
            break;
        default:
            firstSkills.push_back(SKILL_TAILORING);
            firstSkills.push_back(SKILL_ENGINEERING);
            secondSkills.push_back(SKILL_ENCHANTING);
        }

        if (firstSkills.empty() || secondSkills.empty())
        {
            switch (urand(0, 6))
            {
            case 0:
                firstSkill = SKILL_HERBALISM;
                secondSkill = SKILL_ALCHEMY;
                break;
            case 1:
                firstSkill = SKILL_HERBALISM;
                secondSkill = SKILL_MINING;
                break;
            case 2:
                firstSkill = SKILL_MINING;
                secondSkill = SKILL_SKINNING;
                break;
            case 3:
#ifdef MANGOSBOT_ZERO
                firstSkill = SKILL_HERBALISM;
                secondSkill = SKILL_SKINNING;
#else
                firstSkill = SKILL_JEWELCRAFTING;
                secondSkill = SKILL_MINING;
#endif
            }
        }
        else
        {
            firstSkill = firstSkills[urand(0, firstSkills.size() - 1)];
            secondSkill = secondSkills[urand(0, secondSkills.size() - 1)];
        }

        sRandomPlayerbotMgr.SetValue(bot, "firstSkill", firstSkill);
        sRandomPlayerbotMgr.SetValue(bot, "secondSkill", secondSkill);
    }

    SetRandomSkill(SKILL_FIRST_AID);
    SetRandomSkill(SKILL_FISHING);
    SetRandomSkill(SKILL_COOKING);

    SetRandomSkill(firstSkill);
    SetRandomSkill(secondSkill);

#ifndef MANGOSBOT_ZERO
    // skill proficiencies
    switch (bot->getClass())
    {
    case CLASS_WARRIOR:
    case CLASS_PALADIN:
#ifdef MANGOSBOT_TWO
    case CLASS_DEATH_KNIGHT:
#endif
        bot->learnSpell(9788, false);  // armorsmith
        bot->learnSpell(9788, false);  // armorsmith
        bot->learnSpell(9787, false);  // weaponsmith
        bot->learnSpell(17040, false); // hammersmith
        bot->learnSpell(17039, false); // swordsmith
        bot->learnSpell(17041, false); // axesmith
        break;
    }
#endif

    // learn recipies
    for (uint32 id = 0; id < sCreatureStorage.GetMaxEntry(); ++id)
    {
        CreatureInfo const* co = sCreatureStorage.LookupEntry<CreatureInfo>(id);
        if (!co)
            continue;

        if (co->TrainerType != TRAINER_TYPE_TRADESKILLS)
            continue;

        uint32 trainerId = co->TrainerTemplateId;
        if (!trainerId)
            trainerId = co->Entry;

        TrainerSpellData const* trainer_spells = sObjectMgr.GetNpcTrainerTemplateSpells(trainerId);
        if (!trainer_spells)
            trainer_spells = sObjectMgr.GetNpcTrainerSpells(trainerId);

        if (!trainer_spells)
            continue;

        for (TrainerSpellMap::const_iterator itr = trainer_spells->spellList.begin(); itr != trainer_spells->spellList.end(); ++itr)
        {
            TrainerSpell const* tSpell = &itr->second;

            if (!tSpell)
                continue;

            uint32 reqLevel = 0;
            reqLevel = tSpell->isProvidedReqLevel ? tSpell->reqLevel : std::max(reqLevel, tSpell->reqLevel);
            TrainerSpellState state = bot->GetTrainerSpellState(tSpell, reqLevel);
            if (state != TRAINER_SPELL_GREEN)
                continue;

            SpellEntry const* proto = sServerFacade.LookupSpellInfo(tSpell->spell);
            if (!proto)
                continue;

            SpellEntry const* spell = sServerFacade.LookupSpellInfo(tSpell->spell);
            if (spell)
            {
                string SpellName = spell->SpellName[0];
                if (spell->Effect[EFFECT_INDEX_1] == SPELL_EFFECT_SKILL_STEP)
                {
                    uint32 skill = spell->EffectMiscValue[EFFECT_INDEX_1];

                    if (skill && !bot->HasSkill(skill))
                    {
                        SkillLineEntry const* pSkill = sSkillLineStore.LookupEntry(skill);
                        if (pSkill)
                        {
                            if (SpellName.find("Apprentice") != string::npos && pSkill->categoryId == SKILL_CATEGORY_PROFESSION || pSkill->categoryId == SKILL_CATEGORY_SECONDARY)
                                continue;
                        }
                    }
                }
            }

#ifdef CMANGOS
            if (tSpell->learnedSpell)
            {
                bool learned = false;
                for (int j = 0; j < 3; ++j)
                {
                    if (proto->Effect[j] == SPELL_EFFECT_LEARN_SPELL)
                    {
                        uint32 learnedSpell = proto->EffectTriggerSpell[j];
                        bot->learnSpell(learnedSpell, false);
                        learned = true;
                    }
                }
                if (!learned) bot->learnSpell(tSpell->learnedSpell, false);
            }
            else
                ai->CastSpell(tSpell->spell, bot);
#endif

#ifdef MANGOS
            bool learned = false;
            for (int j = 0; j < 3; ++j)
            {
                if (proto->Effect[j] == SPELL_EFFECT_LEARN_SPELL)
                {
                    uint32 learnedSpell = proto->EffectTriggerSpell[j];
                    bot->learnSpell(learnedSpell, false);
                    learned = true;
                }
            }
            if (!learned) bot->learnSpell(tSpell->spell, false);
#endif
        }
    }
}

void PlayerbotFactory::UpdateTradeSkills()
{
    for (int i = 0; i < sizeof(tradeSkills) / sizeof(uint32); ++i)
    {
        if (bot->GetSkillValue(tradeSkills[i]) == 1)
            bot->SetSkill(tradeSkills[i], 0, 0, 0);
    }
}

void PlayerbotFactory::InitSkills()
{
    uint32 maxValue = level * 5;
    bot->UpdateSkillsForLevel(true);

// Riding skills requirements are different
#ifdef MANGOSBOT_ZERO
    if (bot->GetLevel() >= 60)
        bot->SetSkill(SKILL_RIDING, 150, 150);
    else if (bot->GetLevel() >= 40)
        bot->SetSkill(SKILL_RIDING, 75, 75);
#endif
#ifdef MANGOSBOT_ONE
    if (bot->GetLevel() >= 70)
        bot->SetSkill(SKILL_RIDING, 300, 300);
    else if (bot->GetLevel() >= 68)
        bot->SetSkill(SKILL_RIDING, 225, 225);
    else if (bot->GetLevel() >= 60)
        bot->SetSkill(SKILL_RIDING, 150, 150);
    else if (bot->GetLevel() >= 30)
        bot->SetSkill(SKILL_RIDING, 75, 75);
#endif
#ifdef MANGOSBOT_TWO
    if (bot->GetLevel() >= 70)
        bot->SetSkill(SKILL_RIDING, 300, 300);
    else if (bot->GetLevel() >= 60)
        bot->SetSkill(SKILL_RIDING, 225, 225);
    else if (bot->GetLevel() >= 40)
        bot->SetSkill(SKILL_RIDING, 150, 150);
    else if (bot->GetLevel() >= 20)
        bot->SetSkill(SKILL_RIDING, 75, 75);
#endif
    else
        bot->SetSkill(SKILL_RIDING, 0, 0);

    uint32 skillLevel = bot->GetLevel() < 40 ? 0 : 1;
    switch (bot->getClass())
    {
    case CLASS_WARRIOR:
    case CLASS_PALADIN:
        bot->SetSkill(SKILL_PLATE_MAIL, skillLevel, skillLevel);
        break;
    case CLASS_SHAMAN:
    case CLASS_HUNTER:
        bot->SetSkill(SKILL_MAIL, skillLevel, skillLevel);
    }

    switch (bot->getClass())
    {
    case CLASS_DRUID:
        SetRandomSkill(SKILL_MACES);
        SetRandomSkill(SKILL_STAVES);
        SetRandomSkill(SKILL_2H_MACES);
        SetRandomSkill(SKILL_DAGGERS);
        SetRandomSkill(SKILL_POLEARMS);
        SetRandomSkill(SKILL_FIST_WEAPONS);
        break;
    case CLASS_WARRIOR:
        SetRandomSkill(SKILL_SWORDS);
        SetRandomSkill(SKILL_AXES);
        SetRandomSkill(SKILL_BOWS);
        SetRandomSkill(SKILL_GUNS);
        SetRandomSkill(SKILL_MACES);
        SetRandomSkill(SKILL_2H_SWORDS);
        SetRandomSkill(SKILL_STAVES);
        SetRandomSkill(SKILL_2H_MACES);
        SetRandomSkill(SKILL_2H_AXES);
        SetRandomSkill(SKILL_DAGGERS);
        SetRandomSkill(SKILL_CROSSBOWS);
        SetRandomSkill(SKILL_POLEARMS);
        SetRandomSkill(SKILL_FIST_WEAPONS);
        SetRandomSkill(SKILL_THROWN);
        break;
    case CLASS_PALADIN:
        bot->SetSkill(SKILL_PLATE_MAIL, 0, skillLevel, skillLevel);
        SetRandomSkill(SKILL_SWORDS);
        SetRandomSkill(SKILL_AXES);
        SetRandomSkill(SKILL_MACES);
        SetRandomSkill(SKILL_2H_SWORDS);
        SetRandomSkill(SKILL_2H_MACES);
        SetRandomSkill(SKILL_2H_AXES);
        SetRandomSkill(SKILL_POLEARMS);
        break;
    case CLASS_PRIEST:
        SetRandomSkill(SKILL_MACES);
        SetRandomSkill(SKILL_STAVES);
        SetRandomSkill(SKILL_DAGGERS);
        SetRandomSkill(SKILL_WANDS);
        break;
    case CLASS_SHAMAN:
        SetRandomSkill(SKILL_AXES);
        SetRandomSkill(SKILL_MACES);
        SetRandomSkill(SKILL_STAVES);
        SetRandomSkill(SKILL_2H_MACES);
        SetRandomSkill(SKILL_2H_AXES);
        SetRandomSkill(SKILL_DAGGERS);
        SetRandomSkill(SKILL_FIST_WEAPONS);
        break;
    case CLASS_MAGE:
        SetRandomSkill(SKILL_SWORDS);
        SetRandomSkill(SKILL_STAVES);
        SetRandomSkill(SKILL_DAGGERS);
        SetRandomSkill(SKILL_WANDS);
        break;
    case CLASS_WARLOCK:
        SetRandomSkill(SKILL_SWORDS);
        SetRandomSkill(SKILL_STAVES);
        SetRandomSkill(SKILL_DAGGERS);
        SetRandomSkill(SKILL_WANDS);
        break;
    case CLASS_HUNTER:
        SetRandomSkill(SKILL_SWORDS);
        SetRandomSkill(SKILL_AXES);
        SetRandomSkill(SKILL_BOWS);
        SetRandomSkill(SKILL_GUNS);
        SetRandomSkill(SKILL_2H_SWORDS);
        SetRandomSkill(SKILL_STAVES);
        SetRandomSkill(SKILL_2H_AXES);
        SetRandomSkill(SKILL_DAGGERS);
        SetRandomSkill(SKILL_CROSSBOWS);
        SetRandomSkill(SKILL_POLEARMS);
        SetRandomSkill(SKILL_FIST_WEAPONS);
        SetRandomSkill(SKILL_THROWN);
        bot->SetSkill(SKILL_MAIL, 0, skillLevel, skillLevel);
        break;
    case CLASS_ROGUE:
        SetRandomSkill(SKILL_SWORDS);
        SetRandomSkill(SKILL_AXES);
        SetRandomSkill(SKILL_BOWS);
        SetRandomSkill(SKILL_GUNS);
        SetRandomSkill(SKILL_MACES);
        SetRandomSkill(SKILL_DAGGERS);
        SetRandomSkill(SKILL_CROSSBOWS);
        SetRandomSkill(SKILL_FIST_WEAPONS);
        SetRandomSkill(SKILL_THROWN);
        break;
#ifdef MANGOSBOT_TWO
    case CLASS_DEATH_KNIGHT:
        SetRandomSkill(SKILL_SWORDS);
        SetRandomSkill(SKILL_AXES);
        SetRandomSkill(SKILL_MACES);
        SetRandomSkill(SKILL_2H_SWORDS);
        SetRandomSkill(SKILL_2H_MACES);
        SetRandomSkill(SKILL_2H_AXES);
        SetRandomSkill(SKILL_POLEARMS);
        break;
#endif
    }
}

void PlayerbotFactory::SetRandomSkill(uint16 id)
{
    uint32 maxValue = level * 5; // vanilla 60*5 = 300

// do not let skill go beyond limit even if maxlevel > blizzlike
#ifndef MANGOSBOT_ZERO
	if (level > 60)
    {
#ifdef MANGOSBOT_ONE
        maxValue = (level + 5) * 5;   // tbc (70 + 5)*5 = 375
#else
        maxValue = (level + 10) * 5;  // wotlk (80 + 10)*5 = 450
#endif
	}
#endif

    uint32 value = urand(maxValue - level, maxValue);
    uint32 curValue = bot->GetSkillValue(id);
    if (!bot->HasSkill(id) || value > curValue)
        bot->SetSkill(id, value, maxValue);
}

void PlayerbotFactory::InitAvailableSpells()
{
    bot->learnDefaultSpells();
    bot->learnClassLevelSpells(true);

#ifndef MANGOSBOT_TWO
    if (bot->getClass() == CLASS_PALADIN)
    {
        // judgement missing
        if(!bot->HasSpell(20271)) 
        {
            bot->learnSpell(20271, false);
        }
        
        // crusader strike
        if(!bot->HasSpell(33394)) 
        {
            bot->learnSpell(33394, false);
        }

        // hand of reckoning
        if (!bot->HasSpell(33395))
        {
            bot->learnSpell(33395, false);
        }
    }
#endif

    // add polymorph pig/turtle
    if (bot->getClass() == CLASS_MAGE && bot->GetLevel() >= 60)
    {
        bot->learnSpell(28271, false);
        bot->learnSpell(28272, false);
    }

    // add inferno
    if (bot->getClass() == CLASS_WARLOCK && !bot->HasSpell(1122) && bot->GetLevel() >= 50)
        bot->learnSpell(1122, false);

#ifdef MANGOSBOT_ZERO
    // add book spells
    if (bot->GetLevel() == 60)
    {
        vector<uint32> bookSpells;
        switch (bot->getClass())
        {
        case CLASS_WARRIOR:
            bookSpells.push_back(25289);
            bookSpells.push_back(25288);
            bookSpells.push_back(25958);
            break;
        case CLASS_PALADIN:
            bookSpells.push_back(25291);
            bookSpells.push_back(25290);
            bookSpells.push_back(25292);
            break;
        case CLASS_HUNTER:
            bookSpells.push_back(25296);
            bookSpells.push_back(25294);
            bookSpells.push_back(25295);
            break;
        case CLASS_MAGE:
            bookSpells.push_back(23028);
            bookSpells.push_back(25345);
            bookSpells.push_back(25306);
            bookSpells.push_back(3723);
            bookSpells.push_back(28612);
            break;
        case CLASS_ROGUE:
            bookSpells.push_back(25300);
            bookSpells.push_back(25302);
            bookSpells.push_back(31016);
            break;
        case CLASS_PRIEST:
            bookSpells.push_back(25314);
            bookSpells.push_back(25315);
            bookSpells.push_back(25316);
            bookSpells.push_back(21564);
            bookSpells.push_back(27683);
            break;
        case CLASS_SHAMAN:
            bookSpells.push_back(29228);
            bookSpells.push_back(25359);
            bookSpells.push_back(25357);
            bookSpells.push_back(25361);
            break;
        case CLASS_WARLOCK:
            bookSpells.push_back(25311);
            bookSpells.push_back(25309);
            bookSpells.push_back(25307);
            bookSpells.push_back(28610);
            break;
        case CLASS_DRUID:
            bookSpells.push_back(31018);
            bookSpells.push_back(25297);
            bookSpells.push_back(25299);
            bookSpells.push_back(25298);
            bookSpells.push_back(21850);
            break;
        }

        for (auto spellId : bookSpells)
        {
            if (!bot->HasSpell(spellId))
                bot->learnSpell(spellId, false);
        }
    }
#endif
}


void PlayerbotFactory::InitSpecialSpells()
{
    for (list<uint32>::iterator i = sPlayerbotAIConfig.randomBotSpellIds.begin(); i != sPlayerbotAIConfig.randomBotSpellIds.end(); ++i)
    {
        uint32 spellId = *i;

        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);

        if(spellInfo)
            bot->learnSpell(spellId, false);
    }
}

void PlayerbotFactory::InitTalents(uint32 specNo)
{
    uint32 classMask = bot->getClassMask();

    map<uint32, vector<TalentEntry const*> > spells;
    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const *talentInfo = sTalentStore.LookupEntry(i);
        if(!talentInfo)
            continue;

        TalentTabEntry const *talentTabInfo = sTalentTabStore.LookupEntry( talentInfo->TalentTab );
        if(!talentTabInfo || talentTabInfo->tabpage != specNo)
            continue;

        if( (classMask & talentTabInfo->ClassMask) == 0 )
            continue;

        spells[talentInfo->Row].push_back(talentInfo);
    }

    uint32 freePoints = bot->GetFreeTalentPoints();
    for (map<uint32, vector<TalentEntry const*> >::iterator i = spells.begin(); i != spells.end(); ++i)
    {
        vector<TalentEntry const*> &spells = i->second;
        if (spells.empty())
        {
            sLog.outError("%s: No spells for talent row %d", bot->GetName(), i->first);
            continue;
        }

        int attemptCount = 0;
        while (!spells.empty() && (int)freePoints - (int)bot->GetFreeTalentPoints() < 5 && attemptCount++ < 3 && bot->GetFreeTalentPoints())
        {
            int index = urand(0, spells.size() - 1);
            TalentEntry const *talentInfo = spells[index];
            for (int rank = 0; rank < MAX_TALENT_RANK && bot->GetFreeTalentPoints(); ++rank)
            {
                uint32 spellId = talentInfo->RankID[rank];
                if (!spellId)
                    continue;

                bot->learnSpell(spellId, false);
                bot->UpdateFreeTalentPoints(false);
            }
            spells.erase(spells.begin() + index);
        }

        freePoints = bot->GetFreeTalentPoints();
    }
}

ObjectGuid PlayerbotFactory::GetRandomBot()
{
    vector<ObjectGuid> guids;
    for (list<uint32>::iterator i = sPlayerbotAIConfig.randomBotAccounts.begin(); i != sPlayerbotAIConfig.randomBotAccounts.end(); i++)
    {
        uint32 accountId = *i;
        if (!sAccountMgr.GetCharactersCount(accountId))
            continue;

        auto result = CharacterDatabase.PQuery("SELECT guid FROM characters WHERE account = '%u'", accountId);
        if (!result)
            continue;

        do
        {
            Field* fields = result->Fetch();
            ObjectGuid guid = ObjectGuid(fields[0].GetUInt64());
            if (!sObjectMgr.GetPlayer(guid))
                guids.push_back(guid);
        } while (result->NextRow());
    }

    if (guids.empty())
        return ObjectGuid();

    int index = urand(0, guids.size() - 1);
    return guids[index];
}


void PlayerbotFactory::AddPrevQuests(uint32 questId, list<uint32>& questIds)
{
    Quest const *quest = sObjectMgr.GetQuestTemplate(questId);
    for (Quest::PrevQuests::const_iterator iter = quest->prevQuests.begin(); iter != quest->prevQuests.end(); ++iter)
    {
        uint32 prevId = abs(*iter);
        AddPrevQuests(prevId, questIds);
        questIds.remove(prevId);
        questIds.push_back(prevId);
    }
}

void PlayerbotFactory::InitQuests(list<uint32>& questMap)
{
    int count = 0;
    for (list<uint32>::iterator i = questMap.begin(); i != questMap.end(); ++i)
    {
        uint32 questId = *i;
        Quest const *quest = sObjectMgr.GetQuestTemplate(questId);

        if (!bot->SatisfyQuestClass(quest, false) ||
                quest->GetMinLevel() > bot->GetLevel() ||
                !bot->SatisfyQuestRace(quest, false))
            continue;

        bot->SetQuestStatus(questId, QUEST_STATUS_COMPLETE);
        bot->RewardQuest(quest, 0, bot, false);
        sLog.outDetail("Bot %s (%d level) rewarded quest %d: %s (MinLevel=%d, QuestLevel=%d)",
                bot->GetName(), bot->GetLevel(), questId, quest->GetTitle().c_str(),
                quest->GetMinLevel(), quest->GetQuestLevel());
        /*if (!(count++ % 10))
            ClearInventory();*/
    }
}

void PlayerbotFactory::ClearInventory()
{
    DestroyItemsVisitor visitor(bot);
    IterateItemsMask mask = IterateItemsMask((uint8)IterateItemsMask::ITERATE_ITEMS_IN_BAGS | (uint8)IterateItemsMask::ITERATE_ITEMS_IN_EQUIP);
    ai->InventoryIterateItems(&visitor, mask);
}

void PlayerbotFactory::ClearAllItems()
{
    DestroyItemsVisitor visitor(bot);
    ai->InventoryIterateItems(&visitor, IterateItemsMask::ITERATE_ALL_ITEMS);
}

void PlayerbotFactory::InitAmmo()
{
    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_ROGUE && bot->getClass() != CLASS_WARRIOR)
        return;

    Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
    if (!pItem)
        return;

    uint32 subClass = 0;
    switch (pItem->GetProto()->SubClass)
    {
    case ITEM_SUBCLASS_WEAPON_GUN:
        subClass = ITEM_SUBCLASS_BULLET;
        break;
    case ITEM_SUBCLASS_WEAPON_BOW:
    case ITEM_SUBCLASS_WEAPON_CROSSBOW:
        subClass = ITEM_SUBCLASS_ARROW;
        break;
    case ITEM_SUBCLASS_WEAPON_THROWN:
        if (bot->getClass() != CLASS_HUNTER)
        {
            subClass = ITEM_SUBCLASS_THROWN;
            break;
        }
    }

    if (!subClass)
        return;

    uint32 entry = bot->GetUInt32Value(PLAYER_AMMO_ID);
    uint32 count = bot->GetItemCount(entry) / 200;
    uint32 maxCount = 5 + level / 10;

    if (ai->HasCheat(BotCheatMask::item))
        maxCount = 1;

    if (!entry || count <= 2)
    {
        entry = sRandomItemMgr.GetAmmo(level, subClass);
        count = bot->GetItemCount(entry) / 200;
    }

    if (!entry)
        return;

    if (count < maxCount)
    {
        for (uint32 i = 0; i < maxCount - count; i++)
        {
            Item* newItem = bot->StoreNewItemInInventorySlot(entry, 200);
        }
    }

    if(bot->GetUInt32Value(PLAYER_AMMO_ID) != entry)
        bot->SetAmmo(entry);
}

void PlayerbotFactory::InitMounts()
{
    uint32 firstmount =
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

    uint32 secondmount =
#ifdef MANGOSBOT_ZERO
        60
#else
#ifdef MANGOSBOT_ONE
        60
#else
        40
#endif
#endif
        ;

    uint32 thirdmount =
#ifdef MANGOSBOT_ZERO
        90
#else
#ifdef MANGOSBOT_ONE
        68
#else
        60
#endif
#endif
        ;

    uint32 fourthmount =
#ifdef MANGOSBOT_ZERO
        90
#else
#ifdef MANGOSBOT_ONE
        70
#else
        70
#endif
#endif
        ;

    if (bot->GetLevel() < firstmount)
        return;

    map<uint8, map<uint32, vector<uint32> > > mounts;
    vector<uint32> slow, fast, fslow, ffast;
    switch (bot->getRace())
    {
    case RACE_HUMAN:
        slow = { 470, 6648, 458, 472 };
        fast = { 23228, 23227, 23229 };
        break;
    case RACE_ORC:
        slow = { 6654, 6653, 580 };
        fast = { 23250, 23252, 23251 };
        break;
    case RACE_DWARF:
        slow = { 6899, 6777, 6898 };
        fast = { 23238, 23239, 23240 };
        break;
    case RACE_NIGHTELF:
        slow = { 10789, 8394, 10793 };
        fast = { 23221, 23219, 23338 };
        break;
    case RACE_UNDEAD:
        slow = { 17463, 17464, 17462 };
        fast = { 17465, 23246 };
        break;
    case RACE_TAUREN:
        slow = { 18990, 18989 };
        fast = { 23249, 23248, 23247 };
        break;
    case RACE_GNOME:
        slow = { 10969, 17453, 10873, 17454 };
        fast = { 23225, 23223, 23222 };
        break;
    case RACE_TROLL:
        slow = { 8395, 10796, 10799 };
        fast = { 23241, 23242, 23243 };
        break;
#ifndef MANGOSBOT_ZERO
    case RACE_DRAENEI:
        slow = { 34406, 35711, 35710 };
        fast = { 35713, 35712, 35714 };
        break;
    case RACE_BLOODELF:
        slow = { 33660, 35020, 35022, 35018 };
        fast = { 35025, 35025, 35027 };
        break;

    }
    switch (bot->GetTeam())
    {
    case ALLIANCE:
        fslow = { 32235, 32239, 32240 };
        ffast = { 32242, 32289, 32290, 32292 };
        break;
    case HORDE:
        fslow = { 32244, 32245, 32243 };
        ffast = { 32295, 32297, 32246, 32296 };
        break;
#endif
    }
    mounts[bot->getRace()][0] = slow;
    mounts[bot->getRace()][1] = fast;
    mounts[bot->getRace()][2] = fslow;
    mounts[bot->getRace()][3] = ffast;

    for (uint32 type = 0; type < 4; type++)
    {
        if (bot->GetLevel() < secondmount && type == 1)
            continue;

        if (bot->GetLevel() < thirdmount && type == 2)
            continue;

        if (bot->GetLevel() < fourthmount && type == 3)
            continue;

        uint32 index = urand(0, mounts[bot->getRace()][type].size() - 1);
        uint32 spell = mounts[bot->getRace()][type][index];
        if (spell)
        {
            bot->learnSpell(spell, false);
            sLog.outDetail("Bot %d (%d) learned %s mount %d", bot->GetGUIDLow(), bot->GetLevel(), type == 0 ? "slow" : (type == 1 ? "fast" : "flying"), spell);
        }
    }
}

void PlayerbotFactory::InitPotions()
{
    uint32 effects[] = { SPELL_EFFECT_HEAL, SPELL_EFFECT_ENERGIZE };
    for (int i = 0; i < 2; ++i)
    {
        uint32 effect = effects[i];
        FindPotionVisitor visitor(bot, effect);
        ai->InventoryIterateItems(&visitor, IterateItemsMask::ITERATE_ITEMS_IN_BAGS);
        if (!visitor.GetResult().empty()) continue;

        uint32 itemId = sRandomItemMgr.GetRandomPotion(level, effect);
        if (!itemId)
        {
            sLog.outDetail("No potions (type %d) available for bot %s (%d level)", effect, bot->GetName(), bot->GetLevel());
            continue;
        }

        ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
        if (!proto) continue;

        uint32 maxCount = proto->GetMaxStackSize();
        Item* newItem = bot->StoreNewItemInInventorySlot(itemId, urand(maxCount / 2, maxCount));
    }
}

void PlayerbotFactory::InitFood()
{
    uint32 categories[] = { 11, 59 };
    for (int i = 0; i < 2; ++i)
    {
        uint32 category = categories[i];

        FindFoodVisitor visitor(bot, category);
        ai->InventoryIterateItems(&visitor, IterateItemsMask::ITERATE_ITEMS_IN_BAGS);
        if (!visitor.GetResult().empty()) continue;

        uint32 itemId = sRandomItemMgr.GetFood(level, category);
        if (!itemId)
        {
            sLog.outDetail("No food (category %d) available for bot %s (%d level)", category, bot->GetName(), bot->GetLevel());
            continue;
        }
        ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
        if (!proto) continue;

        uint32 maxCount = proto->GetMaxStackSize();
        Item* newItem = bot->StoreNewItemInInventorySlot(itemId, urand(maxCount / 2, maxCount));
   }
}

void PlayerbotFactory::InitReagents()
{
    list<uint32> items;
    uint32 regCount = 1;
    switch (bot->getClass())
    {
    case CLASS_MAGE:
        regCount = 2;
        if (bot->GetLevel() > 11)
            items = { 17056 };
        if (bot->GetLevel() > 19)
            items = { 17056, 17031 };
        if (bot->GetLevel() > 35)
            items = { 17056, 17031, 17032 };
        if (bot->GetLevel() > 55)
            items = { 17056, 17031, 17032, 17020 };
        break;
    case CLASS_DRUID:
        regCount = 2;
        if (bot->GetLevel() > 19)
            items = { 17034 };
        if (bot->GetLevel() > 29)
            items = { 17035 };
        if (bot->GetLevel() > 39)
            items = { 17036 };
        if (bot->GetLevel() > 49)
            items = { 17037, 17021 };
        if (bot->GetLevel() > 59)
            items = { 17038, 17026 };
        if (bot->GetLevel() > 69)
            items = { 22147, 22148 };
        break;
    case CLASS_PALADIN:
        regCount = 3;
        if (bot->GetLevel() > 50)
            items = { 21177 };
        break;
    case CLASS_SHAMAN:
        regCount = 1;
        if (bot->GetLevel() > 22)
            items = { 17057 };
        if (bot->GetLevel() > 28)
            items = { 17057, 17058 };
        if (bot->GetLevel() > 29)
            items = { 17057, 17058, 17030 };
        break;
    case CLASS_WARLOCK:
        regCount = 10;
        if (bot->GetLevel() > 9)
            items = { 6265 };
        if (bot->GetLevel() > 49)
            items = { 6265, 5565 };
        break;
    case CLASS_PRIEST:
        regCount = 3;
        if (bot->GetLevel() > 48)
            items = { 17028 };
        if (bot->GetLevel() > 55)
            items = { 17028, 17029 };
        break;
    case CLASS_ROGUE:
        regCount = 1;
        if (bot->GetLevel() > 21)
            items = { 5140 };
        if (bot->GetLevel() > 33)
            items = { 5140, 5530 };
        break;
    }

    for (list<uint32>::iterator i = items.begin(); i != items.end(); ++i)
    {
        ItemPrototype const* proto = sObjectMgr.GetItemPrototype(*i);
        if (!proto)
        {
            sLog.outError("No reagent (ItemId %d) found for bot %d (Class:%d)", i, bot->GetGUIDLow(), bot->getClass());
            continue;
        }

        uint32 maxCount = proto->GetMaxStackSize();

        QueryItemCountVisitor visitor(*i);
        ai->InventoryIterateItems(&visitor, IterateItemsMask::ITERATE_ITEMS_IN_BAGS);
        if ((uint32)visitor.GetCount() > maxCount) continue;

        uint32 randCount = urand(maxCount / 2, maxCount * regCount);

        Item* newItem = bot->StoreNewItemInInventorySlot(*i, randCount);

        sLog.outDetail("Bot %d got reagent %s x%d", bot->GetGUIDLow(), proto->Name1, randCount);
    }

    for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
    {
        uint32 spellId = itr->first;

        if (itr->second.state == PLAYERSPELL_REMOVED || itr->second.disabled || IsPassiveSpell(spellId))
            continue;

        const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);
        if (!pSpellInfo)
            continue;

        if (pSpellInfo->Effect[0] == SPELL_EFFECT_LEARN_SPELL)
            continue;

        for (const auto& totem : pSpellInfo->Totem)
        {
            if (totem && !bot->HasItemCount(totem, 1))
            {
                ItemPrototype const* proto = sObjectMgr.GetItemPrototype(totem);
                if (!proto)
                {
                    sLog.outError("No totem (ItemId %d) found for bot %d (Class:%d)", totem, bot->GetGUIDLow(), bot->getClass());
                    continue;
                }

                Item* newItem = bot->StoreNewItemInInventorySlot(totem, 1);

                sLog.outDetail("Bot %d got totem %s x%d", bot->GetGUIDLow(), proto->Name1, 1);
            }
        }
#ifndef MANGOSBOT_ZERO
        for (const auto& totemCat : pSpellInfo->TotemCategory)
        {
            if (totemCat && !bot->HasItemTotemCategory(totemCat))
            {
                uint32 itemId = 0;
                if (totemCat == 2)
                    itemId = 5175; // earth totem
                if (totemCat == 3)
                    itemId = 5178; // air totem
                if (totemCat == 4)
                    itemId = 5176; // fire totem
                if (totemCat == 5)
                    itemId = 5177; // water totem

                if (itemId)
                {
                    ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
                    if (!proto)
                    {
                        sLog.outError("No totem (ItemId %d) found for bot %d (Class:%d)", itemId, bot->GetGUIDLow(), bot->getClass());
                        continue;
                    }

                    Item* newItem = bot->StoreNewItemInInventorySlot(itemId, 1);

                    sLog.outDetail("Bot %d got totem %s x%d", bot->GetGUIDLow(), proto->Name1, 1);
                }
            }
        }
#endif
    }
}

void PlayerbotFactory::CancelAuras()
{
    bot->RemoveAllAuras();
}

void PlayerbotFactory::InitInventory()
{
    //InitInventoryTrade();
    //InitInventoryEquip();
    InitInventorySkill();
}

void PlayerbotFactory::InitInventorySkill()
{
    if (bot->HasSkill(SKILL_MINING)) {
        StoreItem(2901, 1); // Mining Pick
    }
    if (bot->HasSkill(SKILL_BLACKSMITHING) || bot->HasSkill(SKILL_ENGINEERING)) {
        StoreItem(5956, 1); // Blacksmith Hammer
    }
    if (bot->HasSkill(SKILL_ENGINEERING)) {
        StoreItem(6219, 1); // Arclight Spanner
    }
    if (bot->HasSkill(SKILL_ENCHANTING)) {
        StoreItem(16207, 1); // Runed Arcanite Rod
    }
    if (bot->HasSkill(SKILL_SKINNING)) {
        StoreItem(7005, 1); // Skinning Knife
    }
}

Item* PlayerbotFactory::StoreItem(uint32 itemId, uint32 count, bool ignoreCount)
{
    if (!ignoreCount)
    {
        if (bot->HasItemCount(itemId, count))
            return nullptr;
    }

    ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
    ItemPosCountVec sDest;
    InventoryResult msg = bot->CanStoreNewItem(INVENTORY_SLOT_BAG_0, NULL_SLOT, sDest, itemId, count);
    if (msg != EQUIP_ERR_OK)
        return NULL;

    return bot->StoreNewItem(sDest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));
}

void PlayerbotFactory::InitInventoryTrade()
{
    uint32 itemId = sRandomItemMgr.GetRandomTrade(level);
    if (!itemId)
    {
        sLog.outError("No trade items available for bot %s (%d level)", bot->GetName(), bot->GetLevel());
        return;
    }

    ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
    if (!proto)
        return;

    uint32 count = 1, stacks = 1;
    switch (proto->Quality)
    {
    case ITEM_QUALITY_NORMAL:
        count = proto->GetMaxStackSize();
        stacks = urand(1, 3) / auctionbot.GetRarityPriceMultiplier(proto);
        break;
    case ITEM_QUALITY_UNCOMMON:
        stacks = 1;
        count = urand(1, proto->GetMaxStackSize() / 2);
        break;
    }

    for (uint32 i = 0; i < stacks; i++)
        StoreItem(itemId, count);
}

void PlayerbotFactory::InitInventoryEquip()
{
    vector<uint32> ids;

    uint32 desiredQuality = ITEM_QUALITY_NORMAL;
    if (level < 10)
        desiredQuality = urand(ITEM_QUALITY_POOR, ITEM_QUALITY_UNCOMMON);
    if (level < 20)
        desiredQuality = urand(ITEM_QUALITY_NORMAL, ITEM_QUALITY_UNCOMMON);
    else if (level < 40)
        desiredQuality = urand(ITEM_QUALITY_UNCOMMON, ITEM_QUALITY_RARE);
    else if (level < 60)
#ifdef MANGOSBOT_ZERO
        desiredQuality = urand(ITEM_QUALITY_UNCOMMON, ITEM_QUALITY_RARE);
    else if (level < 70)
        desiredQuality = urand(ITEM_QUALITY_RARE, ITEM_QUALITY_EPIC);
#else
        desiredQuality = urand(ITEM_QUALITY_UNCOMMON, ITEM_QUALITY_RARE);
    else if (level < 70)
        desiredQuality = urand(ITEM_QUALITY_RARE, ITEM_QUALITY_EPIC);
#endif
    else if (level < 80)
        desiredQuality = urand(ITEM_QUALITY_UNCOMMON, ITEM_QUALITY_RARE);
    else
        desiredQuality = urand(ITEM_QUALITY_RARE, ITEM_QUALITY_EPIC);

    if (urand(0, 100) < 100 * sPlayerbotAIConfig.randomGearLoweringChance && desiredQuality > ITEM_QUALITY_NORMAL) {
        desiredQuality--;
    }

    for (uint32 itemId = 0; itemId < sItemStorage.GetMaxEntry(); ++itemId)
    {
        ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
        if (!proto)
            continue;

        if (proto->ItemLevel > sPlayerbotAIConfig.randomGearMaxLevel)
            continue;

        // do not use items that required level is too low compared to bot's level
        uint32 reqLevel = sRandomItemMgr.GetMinLevelFromCache(itemId);
        if (reqLevel && proto->Quality < ITEM_QUALITY_LEGENDARY && abs((int)bot->GetLevel() - (int)reqLevel) > (int)sPlayerbotAIConfig.randomGearMaxDiff)
            continue;

        if ((proto->Class != ITEM_CLASS_ARMOR && proto->Class != ITEM_CLASS_WEAPON) || (proto->Bonding == BIND_WHEN_PICKED_UP ||
                proto->Bonding == BIND_WHEN_USE))
            continue;


        if (proto->Class == ITEM_CLASS_ARMOR && !CanEquipArmor(proto))
            continue;

        if (proto->Class == ITEM_CLASS_WEAPON && !CanEquipWeapon(proto))
            continue;

        ids.push_back(itemId);
    }

    int maxCount = urand(0, 5);
    int count = 0;
    for (int attempts = 0; attempts < 15; attempts++)
    {
        uint32 index = urand(0, ids.size() - 1);
        uint32 itemId = ids[index];
        Item* newItem = StoreItem(itemId, 1);
        if (newItem && count++ >= maxCount)
            break;
   }
}

void PlayerbotFactory::InitGuild()
{
    // add guild tabard
    if (bot->GetGuildId() && !bot->HasItemCount(5976, 1))
        StoreItem(5976, 1);

    if (bot->GetGuildId())
        return;

    if (sPlayerbotAIConfig.randomBotGuilds.size() < sPlayerbotAIConfig.randomBotGuildCount)
        RandomPlayerbotFactory::CreateRandomGuilds();

    vector<uint32> guilds;
    for (list<uint32>::iterator i = sPlayerbotAIConfig.randomBotGuilds.begin(); i != sPlayerbotAIConfig.randomBotGuilds.end(); ++i)
    {
        Guild* guild = sGuildMgr.GetGuildById(*i);
        if (!guild)
            continue;

        ObjectGuid leaderGuid = guild->GetLeaderGuid();
        if (sObjectMgr.GetPlayerTeamByGUID(leaderGuid) != bot->GetTeam())
            continue;

        guilds.push_back(*i);
    }

    if (guilds.empty())
    {
        sLog.outError("No random guilds available");
        return;
    }

    int index = urand(0, guilds.size() - 1);
    uint32 guildId = guilds[index];
    Guild* guild = sGuildMgr.GetGuildById(guildId);
    if (!guild)
    {
        sLog.outError("Can't join random guild, ID: %u", guildId);
        return;
    }

    uint32 num = atoi(guild->GetGINFO().c_str());
    if ((num && guild->GetMemberSize() < num) || (!num && guild->GetMemberSize() < urand(10, 15)))
    {
        uint32 rankId = urand(GR_OFFICER, GR_INITIATE);
        guild->AddMember(bot->GetObjectGuid(), rankId);
        sLog.outBasic("Bot #%d %s:%d <%s>: Guild <%s> R: %s", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName(), guild->GetName().c_str(), guild->GetRankName(rankId).c_str());
    }

    // add guild tabard
    if (bot->GetGuildId() && bot->GetLevel() > 9 && urand(0, 4) && !bot->HasItemCount(5976, 1))
        StoreItem(5976, 1);
}

void PlayerbotFactory::InitImmersive()
{
    uint32 owner = bot->GetObjectGuid().GetCounter();
    map<Stats, int32> percentMap;

    bool initialized = false;
    for (int i = STAT_STRENGTH; i < MAX_STATS; ++i)
    {
        Stats type = (Stats)i;
        ostringstream name; name << "immersive_stat_" << i;
        uint32 value = sRandomPlayerbotMgr.GetValue(owner, name.str());
        if (value) initialized = true;
        percentMap[type] = value;
    }

    if (!initialized)
    {
        switch (bot->getClass())
        {
        case CLASS_DRUID:
        case CLASS_SHAMAN:
            percentMap[STAT_STRENGTH] = 15;
            percentMap[STAT_INTELLECT] = 10;
            percentMap[STAT_SPIRIT] = 5;
            percentMap[STAT_AGILITY] = 35;
            percentMap[STAT_STAMINA] = 35;
            break;
        case CLASS_PALADIN:
            percentMap[STAT_STRENGTH] = 35;
            percentMap[STAT_INTELLECT] = 10;
            percentMap[STAT_SPIRIT] = 5;
            percentMap[STAT_AGILITY] = 15;
            percentMap[STAT_STAMINA] = 35;
            break;
        case CLASS_WARRIOR:
            percentMap[STAT_STRENGTH] = 30;
            percentMap[STAT_SPIRIT] = 10;
            percentMap[STAT_AGILITY] = 20;
            percentMap[STAT_STAMINA] = 40;
            break;
        case CLASS_ROGUE:
        case CLASS_HUNTER:
            percentMap[STAT_STRENGTH] = 15;
            percentMap[STAT_SPIRIT] = 5;
            percentMap[STAT_AGILITY] = 40;
            percentMap[STAT_STAMINA] = 40;
            break;
        case CLASS_MAGE:
            percentMap[STAT_INTELLECT] = 65;
            percentMap[STAT_SPIRIT] = 5;
            percentMap[STAT_STAMINA] = 30;
            break;
        case CLASS_PRIEST:
            percentMap[STAT_INTELLECT] = 15;
            percentMap[STAT_SPIRIT] = 55;
            percentMap[STAT_STAMINA] = 30;
            break;
        case CLASS_WARLOCK:
            percentMap[STAT_INTELLECT] = 30;
            percentMap[STAT_SPIRIT] = 15;
            percentMap[STAT_STAMINA] = 55;
            break;
        }

        for (int i = 0; i < 5; i++)
        {
            Stats from = (Stats)urand(STAT_STRENGTH, MAX_STATS - 1);
            Stats to = (Stats)urand(STAT_STRENGTH, MAX_STATS - 1);
            int32 delta = urand(0, 5 + bot->GetLevel() / 3);
            if (from != to && percentMap[to] + delta <= 100 && percentMap[from] - delta >= 0)
            {
                percentMap[to] += delta;
                percentMap[from] -= delta;
            }
        }

        for (int i = STAT_STRENGTH; i < MAX_STATS; ++i)
        {
            Stats type = (Stats)i;
            ostringstream name; name << "immersive_stat_" << i;
            sRandomPlayerbotMgr.SetValue(owner, name.str(), percentMap[type]);
        }
    }
    bot->InitStatsForLevel(true);
    bot->UpdateAllStats();
}

#ifndef MANGOSBOT_ZERO
void PlayerbotFactory::InitArenaTeam()
{
    if (!sPlayerbotAIConfig.IsInRandomAccountList(bot->GetSession()->GetAccountId()))
        return;

    if (sPlayerbotAIConfig.randomBotArenaTeams.size() < sPlayerbotAIConfig.randomBotArenaTeamCount)
        RandomPlayerbotFactory::CreateRandomArenaTeams();
}
#endif

void PlayerbotFactory::EnchantEquipment()
{
    if (bot->GetLevel() >= sPlayerbotAIConfig.minEnchantingBotLevel)
    {
        if (m_EnchantContainer.empty())
        {
            LoadEnchantContainer();
        }

        for (uint8 slot = 0; slot < SLOT_EMPTY; slot++)
        {
            Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
            if (item)
            {
                EnchantItem(item);
            }
        }
    }
}

void PlayerbotFactory::ApplyEnchantTemplate()
{
   int tab = AiFactory::GetPlayerSpecTab(bot);

   switch (bot->getClass())
   {
   case CLASS_WARRIOR:
      if (tab == 2)
          ApplyEnchantTemplate(12);
      else if (tab == 1)
          ApplyEnchantTemplate(11);
      else
          ApplyEnchantTemplate(10);
      break;
   case CLASS_DRUID:
      if (tab == 2)    
          ApplyEnchantTemplate(112);
      else if (tab == 0) 
          ApplyEnchantTemplate(110);
      else 
          ApplyEnchantTemplate(111);
      break;
   case CLASS_SHAMAN:
      if (tab == 0)
         ApplyEnchantTemplate(70);
      else if (tab == 2)
         ApplyEnchantTemplate(71);
      else
         ApplyEnchantTemplate(72);
      break;
   case CLASS_PALADIN:
      if (tab == 0)
         ApplyEnchantTemplate(20);
      else if (tab == 2)
         ApplyEnchantTemplate(22);
      else if (tab == 1)
         ApplyEnchantTemplate(21);
      break;
   case CLASS_HUNTER:
      ApplyEnchantTemplate(30);
      break;
   case CLASS_ROGUE:
      ApplyEnchantTemplate(40);
      break;
   case CLASS_MAGE:
      ApplyEnchantTemplate(80);
      break;
   case CLASS_WARLOCK:
      ApplyEnchantTemplate(90);
      break;
   case CLASS_PRIEST:
       ApplyEnchantTemplate(50);
       break;
   }
}

void PlayerbotFactory::ApplyEnchantTemplate(uint8 spec, Item* item)
{
   for (EnchantContainer::const_iterator itr = GetEnchantContainerBegin(); itr != GetEnchantContainerEnd(); ++itr)
      if ((*itr)->ClassId == bot->getClass() && (*itr)->SpecId == spec)
         ai->EnchantItemT((*itr)->SpellId, (*itr)->SlotId, item);
}

void PlayerbotFactory::LoadEnchantContainer()
{
   for (EnchantContainer::const_iterator itr = m_EnchantContainer.begin(); itr != m_EnchantContainer.end(); ++itr)
      delete *itr;

   m_EnchantContainer.clear();

   uint32 count = 0;

   auto result = PlayerbotDatabase.PQuery("SELECT class, spec, spellid, slotid FROM ai_playerbot_enchants");
   if (result)
   {
      do
      {
         Field* fields = result->Fetch();

         EnchantTemplate* pEnchant = new EnchantTemplate;

         pEnchant->ClassId = fields[0].GetUInt8();
         pEnchant->SpecId = fields[1].GetUInt8();
         pEnchant->SpellId = fields[2].GetUInt32();
         pEnchant->SlotId = fields[3].GetUInt8();

         m_EnchantContainer.push_back(pEnchant);
         ++count;
      } while (result->NextRow());
   }
}

/*void PlayerbotFactory::InitGems() //WIP
{
#ifndef MANGOSBOT_ZERO
    vector<uint32> gems = sRandomItemMgr.GetGemsList();
    if (!gems.empty()) ahbot::Shuffle(gems);

    for (int slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; slot++)
    {
        if (Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            if (ItemPrototype const* proto = item->GetProto())
            {
                uint32 gem_placed[MAX_GEM_SOCKETS];
                ObjectGuid gem_guids[MAX_GEM_SOCKETS];
                bool gemCreated = false;
                bool hasSockets = false;

                for (int i = 0; i < MAX_GEM_SOCKETS; ++i)               // check for hack maybe
                {
                    // tried to put gem in socket where no socket exists
                    if (!proto->Socket[i].Color)
                        continue;

                    if (proto->Socket[i].Color)
                        hasSockets = true;
                }
                if (!hasSockets)
                    continue;

                    for (int i = 0; i < MAX_GEM_SOCKETS; i++) gem_placed[i] = 0;
                    {
                        for (uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT + MAX_GEM_SOCKETS; ++enchant_slot)
                        {
                        ObjectGuid gem_GUID;
                        uint32 SocketColor = proto->Socket[enchant_slot - SOCK_ENCHANTMENT_SLOT].Color;
                        uint32 gem_id = 0;
                        switch (SocketColor) {
                        case SOCKET_COLOR_META:
                            gem_id = 25890;
                            break;
                        default:
                        {
                            for (vector<uint32>::const_iterator itr = gems.begin(); itr != gems.end(); itr++)
                            {
                                if (ItemPrototype const* gemProto = sObjectMgr.GetItemPrototype(*itr))
                                {
                                    if (GemPropertiesEntry const* gemProperty = sGemPropertiesStore.LookupEntry(gemProto->GemProperties))
                                    {
                                        if (SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(gemProperty->spellitemenchantement))
                                        {
                                            uint32 GemColor = gemProperty->color;

                                            // check unique-equipped on item
                                            if (gemProto->Flags & ITEM_FLAG_UNIQUE_EQUIPPABLE)
                                            {
                                                // there is an equip limit on this item and also we dont want to have more than 1 anywhere
                                                if ((bot->HasItemOrGemWithIdEquipped(gemProto->ItemId, 1)) || (bot->HasItemCount(gemProto->ItemId, 1)))
                                                    continue;
                                            }

                                            if (gemProto->RequiredSkillRank > bot->GetSkillValue(SKILL_JEWELCRAFTING))
                                                continue;

                                            // no nedd epic gems to low gear and never need crap gem to epic gear
                                            if (((proto->ItemLevel) < 100) && ((gemProto->Quality) > 3) || ((proto->ItemLevel) > 100) && ((gemProto->Quality) < 3))
                                                continue;

                                            uint8 sp = 0, ap = 0, tank = 0;
                                            if (GemColor & SocketColor && GemColor == SocketColor)
                                            {
                                                for (int i = 0; i < 3; ++i)
                                                {
                                                    if (pEnchant->type[i] != ITEM_ENCHANTMENT_TYPE_STAT)
                                                        continue;

                                                    AddItemStats(pEnchant->spellid[i], sp, ap, tank);
                                                }
                                            }

                                            if (!CheckItemStats(sp, ap, tank))
                                                continue;
                                       
                                            gem_id = gemProto->ItemId;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        break;
                        }
                        if (gem_id > 0)
                        {
                            gem_placed[enchant_slot - SOCK_ENCHANTMENT_SLOT] = gem_id;
                            if (Item* gem = StoreItem(gem_id, 1))
                            {
                                gem_GUID = gem->GetObjectGuid();
                                gem_guids[enchant_slot - SOCK_ENCHANTMENT_SLOT] = gem_GUID;
                                gemCreated = true;
                            }
                        }
                        }
                        if (gemCreated)
                        {
                        std::unique_ptr<WorldPacket> packet(new WorldPacket(CMSG_SOCKET_GEMS));
                        *packet << item->GetObjectGuid();
                        for (int i = 0; i < MAX_GEM_SOCKETS; ++i)
                        {
                            *packet << gem_guids[i];
                        }
                        bot->GetSession()->QueuePacket(std::move(packet));
                        }
                    }
            }
        }
    }
#endif
}*/
void PlayerbotFactory::InitGems() //WIP
{
#ifndef MANGOSBOT_ZERO
    vector<uint32> gems = sRandomItemMgr.GetGemsList();
    for (int slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; slot++)
    {
        if (Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            if (ItemPrototype const* proto = item->GetProto())
            {
               /*bool hasSockets = false;

                for (int i = 0; i < MAX_GEM_SOCKETS; ++i)               // check for hack maybe
                {
                    // tried to put gem in socket where no socket exists
                    if (!proto->Socket[i].Color)
                        continue;

                    if (proto->Socket[i].Color)
                        hasSockets = true;
                }
                if (!hasSockets)
                    continue;*/


                WorldPacket data(CMSG_SOCKET_GEMS);

                data << item->GetObjectGuid();
                uint32 gem_placed[MAX_GEM_SOCKETS];

                for (int i = 0; i < MAX_GEM_SOCKETS; i++) gem_placed[i] = 0;
                for (uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT + MAX_GEM_SOCKETS; ++enchant_slot)
                {
                    ObjectGuid gem_GUID;
                    //uint64 gem_GUID = 0;
                    uint32 SocketColor = proto->Socket[enchant_slot - SOCK_ENCHANTMENT_SLOT].Color;
                    //uint32 SocketContent = proto->Socket[enchant_slot - SOCK_ENCHANTMENT_SLOT].Content;
                    uint32 gem_id = 0;
                    switch (SocketColor) {
                    case SOCKET_COLOR_META:
                        gem_id = 25890;
                        break;
                    default:
                    {
                        for (vector<uint32>::const_iterator itr = gems.begin(); itr != gems.end(); itr++)
                        {
                            if (ItemPrototype const* gemProto = sObjectMgr.GetItemPrototype(*itr))
                            {
                                // We don't want the same gem twice on the same piece 
                                bool already_placed = false;
                                for (int i = 0; i < MAX_GEM_SOCKETS; i++)
                                    if (gem_placed[i] == gemProto->ItemId)
                                    {
                                        already_placed = true;
                                    }
                                if (already_placed) continue;

                                if (GemPropertiesEntry const* gemProperty = sGemPropertiesStore.LookupEntry(gemProto->GemProperties))
                                {
                                    if (SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(gemProperty->spellitemenchantement))
                                    {
                                        uint32 GemColor = gemProperty->color;

                                      // check unique-equipped on item
                                        if (gemProto->Flags & ITEM_FLAG_UNIQUE_EQUIPPABLE)
                                        {
                                            // there is an equip limit on this item and also we dont want to have more than 1 anywhere
                                            if ((bot->HasItemOrGemWithIdEquipped(gemProto->ItemId, 1)) || (bot->HasItemCount(gemProto->ItemId, 1)))
                                                continue;
                                        }

                                        if (gemProto->RequiredSkillRank > bot->GetSkillValue(SKILL_JEWELCRAFTING))
                                            continue;

                                        // no need epic gems to low gear and never need crap gem to epic gear
                                        if (((proto->ItemLevel) < 100) && ((gemProto->Quality) > 3) || ((proto->ItemLevel) > 100) && ((gemProto->Quality) < 3))
                                            continue;

                                        uint8 sp = 0, ap = 0, tank = 0;
                                        if (GemColor & SocketColor && GemColor == SocketColor)
                                        {
                                            for (int i = 0; i < 3; ++i)
                                            {
                                                if (pEnchant->type[i] != ITEM_ENCHANTMENT_TYPE_STAT && pEnchant->type[i] != ITEM_ENCHANTMENT_TYPE_EQUIP_SPELL)
                                                    continue;
                                                switch (pEnchant->type[i]) 
                                                {
                                                case ITEM_ENCHANTMENT_TYPE_STAT:                                                    
                                                        AddItemStats(pEnchant->spellid[i], sp, ap, tank);
                                                break;                                                    
                                                case ITEM_ENCHANTMENT_TYPE_EQUIP_SPELL:                                                   
                                                        const SpellEntry* const spellInfo = sServerFacade.LookupSpellInfo(pEnchant->spellid[i]);
                                                        if (!spellInfo)
                                                            continue;

                                                        for (int j = 0; j < MAX_EFFECT_INDEX; j++)
                                                        {
                                                            if (spellInfo->Effect[j] != SPELL_EFFECT_APPLY_AURA)
                                                                continue;

                                                            AddItemSpellStats(spellInfo->EffectApplyAuraName[j], sp, ap, tank);
                                                        }  
                                                break;
                                                }
                                            }
                                        }
                                        if (!CheckItemStats(sp, ap, tank))
                                            continue;
                                        gem_id = gemProto->ItemId;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    break;
                    }
                    if (gem_id > 0)
                    {
                        gem_placed[enchant_slot - SOCK_ENCHANTMENT_SLOT] = gem_id;


                        ItemPosCountVec dest;
                        InventoryResult res = bot->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, gem_id, 1);
                        if (res == EQUIP_ERR_OK)
                        {
                            if (Item* gem = bot->StoreNewItem(dest, gem_id, true))
                            {
                                bot->SendNewItem(gem, 1, false, true, false);
                                gem_GUID = gem->GetObjectGuid();

                            }
                        }
                    }
                    data << gem_GUID;
                }
                bot->GetSession()->HandleSocketOpcode(data);
            }
        }
    }
#endif
}

void PlayerbotFactory::InitTaxiNodes()
{
    uint32 startMap = bot->GetMapId();

    if (startMap == 530) //BE=EK, DREA=KAL
        startMap = bot->GetTeam() == ALLIANCE ? 1 : 0;

    TaxiNodeLevelContainer const& overworldTaxiNodeLevels = bot->GetTeam() == ALLIANCE ? overworldTaxiNodeLevelsA : overworldTaxiNodeLevelsH;

    for (TaxiNodeLevelContainer::const_iterator itr = overworldTaxiNodeLevels.begin(); itr != overworldTaxiNodeLevels.end(); ++itr)
    {
        TaxiNodeLevel const& taxiNodeLevel = *itr;

        if (taxiNodeLevel.MapId == 571 && bot->GetLevel() < 66) //Don't learn nodes in northrend before level 66.
            continue;

        if (taxiNodeLevel.MapId == 530 && bot->GetLevel() < 58) //Don't learn nodes in outland before level 58.
            continue;

        if (taxiNodeLevel.Level > bot->GetLevel() && urand(0, 20)) //Limit nodes in high level area's.
            continue;

        if (taxiNodeLevel.MapId != startMap && taxiNodeLevel.Level + 20 > bot->GetLevel() && urand(0, 4)) //Limit nodes on other map.
            continue;

        bot->m_taxi.SetTaximaskNode(taxiNodeLevel.Index);
    }
}
