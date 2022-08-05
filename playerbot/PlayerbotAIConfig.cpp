#include "../botpch.h"
#include "../ahbot/AhBot.h"
#include "PlayerbotAIConfig.h"
#include "playerbot.h"
#include "RandomPlayerbotFactory.h"
#include "AccountMgr.h"
#include "SystemConfig.h"
#include "PlayerbotFactory.h"
#include "RandomItemMgr.h"
#include "World/WorldState.h"

#include "TravelMgr.h"

#include <iostream>

using namespace std;

INSTANTIATE_SINGLETON_1(PlayerbotAIConfig);

PlayerbotAIConfig::PlayerbotAIConfig()
{
}

template <class T>
void LoadList(string value, T &list)
{
    vector<string> ids = split(value, ',');
    for (vector<string>::iterator i = ids.begin(); i != ids.end(); i++)
    {
        string string = *i;
        if (string.empty())
            continue;

        uint32 id = atoi(string.c_str());

        list.push_back(id);
    }
}

template <class T>
void LoadListString(string value, T& list)
{
    vector<string> strings = split(value, ',');
    for (vector<string>::iterator i = strings.begin(); i != strings.end(); i++)
    {
        string string = *i;
        if (string.empty())
            continue;

        list.push_back(string);
    }
}

void PlayerbotAIConfig::loadConfigParameters()
{
    globalCoolDown = (uint32)config.GetIntDefault("AiPlayerbot.GlobalCooldown", 500);
    maxWaitForMove = config.GetIntDefault("AiPlayerbot.MaxWaitForMove", 3000);
    expireActionTime = config.GetIntDefault("AiPlayerbot.ExpireActionTime", 5000);
    dispelAuraDuration = config.GetIntDefault("AiPlayerbot.DispelAuraDuration", 2000);
    reactDelay = (uint32)config.GetIntDefault("AiPlayerbot.ReactDelay", 100);
    passiveDelay = (uint32)config.GetIntDefault("AiPlayerbot.PassiveDelay", 4000);
    repeatDelay = (uint32)config.GetIntDefault("AiPlayerbot.RepeatDelay", 5000);
    errorDelay = (uint32)config.GetIntDefault("AiPlayerbot.ErrorDelay", 5000);
    rpgDelay = (uint32)config.GetIntDefault("AiPlayerbot.RpgDelay", 3000);
    sitDelay = (uint32)config.GetIntDefault("AiPlayerbot.SitDelay", 30000);
    returnDelay = (uint32)config.GetIntDefault("AiPlayerbot.ReturnDelay", 7000);
    lootDelay = (uint32)config.GetIntDefault("AiPlayerbot.LootDelayDelay", 750);

    farDistance = config.GetFloatDefault("AiPlayerbot.FarDistance", 20.0f);
    sightDistance = config.GetFloatDefault("AiPlayerbot.SightDistance", 75.0f);
    spellDistance = config.GetFloatDefault("AiPlayerbot.SpellDistance", 25.0f);
    shootDistance = config.GetFloatDefault("AiPlayerbot.ShootDistance", 25.0f);
    healDistance = config.GetFloatDefault("AiPlayerbot.HealDistance", 25.0f);
    reactDistance = config.GetFloatDefault("AiPlayerbot.ReactDistance", 150.0f);
    grindDistance = config.GetFloatDefault("AiPlayerbot.GrindDistance", 75.0f);
    aggroDistance = config.GetFloatDefault("AiPlayerbot.AggroDistance", 22.0f);
    lootDistance = config.GetFloatDefault("AiPlayerbot.LootDistance", 15.0f);
    fleeDistance = config.GetFloatDefault("AiPlayerbot.FleeDistance", 7.5f);
    tooCloseDistance = config.GetFloatDefault("AiPlayerbot.TooCloseDistance", 5.0f);
    meleeDistance = config.GetFloatDefault("AiPlayerbot.MeleeDistance", 1.5f);
    followDistance = config.GetFloatDefault("AiPlayerbot.FollowDistance", 1.5f);
    whisperDistance = config.GetFloatDefault("AiPlayerbot.WhisperDistance", 6000.0f);
    contactDistance = config.GetFloatDefault("AiPlayerbot.ContactDistance", 0.5f);
    aoeRadius = config.GetFloatDefault("AiPlayerbot.AoeRadius", 5.0f);
    rpgDistance = config.GetFloatDefault("AiPlayerbot.RpgDistance", 80.0f);

    criticalHealth = config.GetIntDefault("AiPlayerbot.CriticalHealth", 20);
    lowHealth = config.GetIntDefault("AiPlayerbot.LowHealth", 50);
    mediumHealth = config.GetIntDefault("AiPlayerbot.MediumHealth", 70);
    almostFullHealth = config.GetIntDefault("AiPlayerbot.AlmostFullHealth", 90);
    lowMana = config.GetIntDefault("AiPlayerbot.LowMana", 15);
    mediumMana = config.GetIntDefault("AiPlayerbot.MediumMana", 40);

    randomGearMaxLevel = config.GetIntDefault("AiPlayerbot.RandomGearMaxLevel", 500);
    randomGearMaxDiff = config.GetIntDefault("AiPlayerbot.RandomGearMaxDiff", 5);
    randomGearProgression = config.GetBoolDefault("AiPlayerbot.RandomGearProgression", true);
    randomGearLoweringChance = config.GetFloatDefault("AiPlayerbot.RandomGearLoweringChance", 0.15f);
    randomBotMaxLevelChance = config.GetFloatDefault("AiPlayerbot.RandomBotMaxLevelChance", 0.15f);
    randomBotRpgChance = config.GetFloatDefault("AiPlayerbot.RandomBotRpgChance", 0.35f);

    iterationsPerTick = config.GetIntDefault("AiPlayerbot.IterationsPerTick", 100);

    allowGuildBots = config.GetBoolDefault("AiPlayerbot.AllowGuildBots", true);

    randomBotMapsAsString = config.GetStringDefault("AiPlayerbot.RandomBotMaps", "0,1,530,571");
    LoadList<vector<uint32> >(randomBotMapsAsString, randomBotMaps);
    LoadList<list<uint32> >(config.GetStringDefault("AiPlayerbot.RandomBotQuestItems", "6948,5175,5176,5177,5178,16309,12382,13704,11000,22754"), randomBotQuestItems);
    LoadList<list<uint32> >(config.GetStringDefault("AiPlayerbot.RandomBotSpellIds", "54197"), randomBotSpellIds);
    LoadList<list<uint32> >(config.GetStringDefault("AiPlayerbot.PvpProhibitedZoneIds", "2255,656,2361,2362,2363,976,35,2268,3425,392,541,1446,3828,3712,3738,3565,3539,3623,4152,3988,4658,4284,4418,4436,4275,4323"), pvpProhibitedZoneIds);

#ifndef MANGOSBOT_ZERO
    // disable pvp near dark portal if event is active
    if (sWorldState.GetExpansion() == EXPANSION_NONE)
        pvpProhibitedZoneIds.insert(pvpProhibitedZoneIds.begin(), 72);
#endif

    LoadList<list<uint32> >(config.GetStringDefault("AiPlayerbot.RandomBotQuestIds", "7848,3802,5505,6502,7761"), randomBotQuestIds);

    botAutologin = config.GetBoolDefault("AiPlayerbot.BotAutologin", false);
    randomBotAutologin = config.GetBoolDefault("AiPlayerbot.RandomBotAutologin", true);
    minRandomBots = config.GetIntDefault("AiPlayerbot.MinRandomBots", 50);
    maxRandomBots = config.GetIntDefault("AiPlayerbot.MaxRandomBots", 200);
    randomBotUpdateInterval = config.GetIntDefault("AiPlayerbot.RandomBotUpdateInterval", 60);
    randomBotCountChangeMinInterval = config.GetIntDefault("AiPlayerbot.RandomBotCountChangeMinInterval", 1 * 1800);
    randomBotCountChangeMaxInterval = config.GetIntDefault("AiPlayerbot.RandomBotCountChangeMaxInterval", 2 * 3600);
    minRandomBotInWorldTime = config.GetIntDefault("AiPlayerbot.MinRandomBotInWorldTime", 1 * 1800);
    maxRandomBotInWorldTime = config.GetIntDefault("AiPlayerbot.MaxRandomBotInWorldTime", 6 * 3600);
    minRandomBotRandomizeTime = config.GetIntDefault("AiPlayerbot.MinRandomBotRandomizeTime", 3600);
    maxRandomBotRandomizeTime = config.GetIntDefault("AiPlayerbot.MaxRandomRandomizeTime", 6 * 3600);
    minRandomBotChangeStrategyTime = config.GetIntDefault("AiPlayerbot.MinRandomBotChangeStrategyTime", 1800);
    maxRandomBotChangeStrategyTime = config.GetIntDefault("AiPlayerbot.MaxRandomBotChangeStrategyTime", 2 * 3600);
    minRandomBotReviveTime = config.GetIntDefault("AiPlayerbot.MinRandomBotReviveTime", 60);
    maxRandomBotReviveTime = config.GetIntDefault("AiPlayerbot.MaxRandomReviveTime", 300);
    randomBotTeleportDistance = config.GetIntDefault("AiPlayerbot.RandomBotTeleportDistance", 1000);
    randomBotsPerInterval = config.GetIntDefault("AiPlayerbot.RandomBotsPerInterval", 50);
    minRandomBotsPriceChangeInterval = config.GetIntDefault("AiPlayerbot.MinRandomBotsPriceChangeInterval", 2 * 3600);
    maxRandomBotsPriceChangeInterval = config.GetIntDefault("AiPlayerbot.MaxRandomBotsPriceChangeInterval", 48 * 3600);
    randomBotJoinLfg = config.GetBoolDefault("AiPlayerbot.RandomBotJoinLfg", true);
    randomBotJoinBG = config.GetBoolDefault("AiPlayerbot.RandomBotJoinBG", true);
    randomBotAutoJoinBG = config.GetBoolDefault("AiPlayerbot.RandomBotAutoJoinBG", false);
    randomBotBracketCount = config.GetIntDefault("AiPlayerbot.RandomBotBracketCount", 3);
    logInGroupOnly = config.GetBoolDefault("AiPlayerbot.LogInGroupOnly", true);
    logValuesPerTick = config.GetBoolDefault("AiPlayerbot.LogValuesPerTick", false);
    fleeingEnabled = config.GetBoolDefault("AiPlayerbot.FleeingEnabled", true);
    summonAtInnkeepersEnabled = config.GetBoolDefault("AiPlayerbot.SummonAtInnkeepersEnabled", true);
    randomBotMinLevel = config.GetIntDefault("AiPlayerbot.RandomBotMinLevel", 1);
    randomBotMaxLevel = config.GetIntDefault("AiPlayerbot.RandomBotMaxLevel", 255);
    randomBotLoginAtStartup = config.GetBoolDefault("AiPlayerbot.RandomBotLoginAtStartup", true);
    randomBotTeleLevel = config.GetIntDefault("AiPlayerbot.RandomBotTeleLevel", 5);
    openGoSpell = config.GetIntDefault("AiPlayerbot.OpenGoSpell", 6477);

    randomChangeMultiplier = config.GetFloatDefault("AiPlayerbot.RandomChangeMultiplier", 1.0);

    randomBotCombatStrategies = config.GetStringDefault("AiPlayerbot.RandomBotCombatStrategies", "-threat,+custom::say");
    randomBotNonCombatStrategies = config.GetStringDefault("AiPlayerbot.RandomBotNonCombatStrategies", "+custom::say");
    combatStrategies = config.GetStringDefault("AiPlayerbot.CombatStrategies", "");
    nonCombatStrategies = config.GetStringDefault("AiPlayerbot.NonCombatStrategies", "+return");

    commandPrefix = config.GetStringDefault("AiPlayerbot.CommandPrefix", "");
    commandSeparator = config.GetStringDefault("AiPlayerbot.CommandSeparator", "\\\\");

    commandServerPort = config.GetIntDefault("AiPlayerbot.CommandServerPort", 0);
    perfMonEnabled = config.GetBoolDefault("AiPlayerbot.PerfMonEnabled", false);

    randomBotAccountPrefix = config.GetStringDefault("AiPlayerbot.RandomBotAccountPrefix", "rndbot");
    randomBotAccountCount = config.GetIntDefault("AiPlayerbot.RandomBotAccountCount", 50);
    deleteRandomBotAccounts = config.GetBoolDefault("AiPlayerbot.DeleteRandomBotAccounts", false);
    randomBotGuildCount = config.GetIntDefault("AiPlayerbot.RandomBotGuildCount", 20);
    deleteRandomBotGuilds = config.GetBoolDefault("AiPlayerbot.DeleteRandomBotGuilds", false);

    //arena
    randomBotArenaTeamCount = config.GetIntDefault("AiPlayerbot.RandomBotArenaTeamCount", 20);
    deleteRandomBotArenaTeams = config.GetBoolDefault("AiPlayerbot.DeleteRandomBotArenaTeams", false);

    guildTaskEnabled = config.GetBoolDefault("AiPlayerbot.EnableGuildTasks", true);
    minGuildTaskChangeTime = config.GetIntDefault("AiPlayerbot.MinGuildTaskChangeTime", 3 * 24 * 3600);
    maxGuildTaskChangeTime = config.GetIntDefault("AiPlayerbot.MaxGuildTaskChangeTime", 4 * 24 * 3600);
    minGuildTaskAdvertisementTime = config.GetIntDefault("AiPlayerbot.MinGuildTaskAdvertisementTime", 60);
    maxGuildTaskAdvertisementTime = config.GetIntDefault("AiPlayerbot.MaxGuildTaskAdvertisementTime", 12 * 3600);
    minGuildTaskRewardTime = config.GetIntDefault("AiPlayerbot.MinGuildTaskRewardTime", 30);
    maxGuildTaskRewardTime = config.GetIntDefault("AiPlayerbot.MaxGuildTaskRewardTime", 120);
    guildTaskAdvertCleanupTime = config.GetIntDefault("AiPlayerbot.GuildTaskAdvertCleanupTime", 300);

    //cosmetics (by lidocain)
    randomBotShowCloak = config.GetBoolDefault("AiPlayerbot.RandomBotShowCloak", false);
    randomBotShowHelmet = config.GetBoolDefault("AiPlayerbot.RandomBotShowHelmet", false);

    //SPP switches
    enableGreet = config.GetBoolDefault("AiPlayerbot.EnableGreet", false);
    disableRandomLevels = config.GetBoolDefault("AiPlayerbot.DisableRandomLevels", false);
    randomBotRandomPassword = config.GetBoolDefault("AiPlayerbot.RandomBotRandomPassword", true);
    playerbotsXPrate = config.GetIntDefault("AiPlayerbot.KillXPRate", 1);
    botActiveAlone = config.GetIntDefault("AiPlayerbot.botActiveAlone", 10); //hidden config
    RandombotsWalkingRPG = config.GetBoolDefault("AiPlayerbot.RandombotsWalkingRPG", false);
    RandombotsWalkingRPGInDoors = config.GetBoolDefault("AiPlayerbot.RandombotsWalkingRPG.InDoors", false);
    minEnchantingBotLevel = config.GetIntDefault("AiPlayerbot.minEnchantingBotLevel", 60);
    randombotStartingLevel = config.GetIntDefault("AiPlayerbot.randombotStartingLevel", 5);
    gearscorecheck = config.GetBoolDefault("AiPlayerbot.GearScoreCheck", false);
    randomBotPreQuests = config.GetBoolDefault("AiPlayerbot.PreQuests", true);
    randomBotSayWithoutMaster = config.GetBoolDefault("AiPlayerbot.RandomBotSayWithoutMaster", false);
    randomBotGroupNearby = config.GetBoolDefault("AiPlayerbot.RandomBotGroupNearby", false);

    //SPP automation
    autoPickReward = config.GetStringDefault("AiPlayerbot.AutoPickReward", "no");
    autoEquipUpgradeLoot = config.GetBoolDefault("AiPlayerbot.AutoEquipUpgradeLoot", false);
    syncQuestWithPlayer = config.GetBoolDefault("AiPlayerbot.SyncQuestWithPlayer", false);
    syncQuestForPlayer = config.GetBoolDefault("AiPlayerbot.SyncQuestForPlayer", false);
    autoTrainSpells = config.GetStringDefault("AiPlayerbot.AutoTrainSpells", "no");
    autoPickTalents = config.GetStringDefault("AiPlayerbot.AutoPickTalents", "no");
    autoLearnTrainerSpells = config.GetBoolDefault("AiPlayerbot.AutoLearnTrainerSpells", false);
    autoLearnQuestSpells = config.GetBoolDefault("AiPlayerbot.AutoLearnQuestSpells", false);
    autoDoQuests = config.GetBoolDefault("AiPlayerbot.AutoDoQuests", false);
    syncLevelWithPlayers = config.GetBoolDefault("AiPlayerbot.SyncLevelWithPlayers", false);
    tweakValue = config.GetIntDefault("AiPlayerbot.TweakValue", 0);
    freeFood = config.GetBoolDefault("AiPlayerbot.FreeFood", true);

    selfBotLevel = config.GetIntDefault("AiPlayerbot.SelfBotLevel", 1);

    targetPosRecalcDistance = config.GetFloatDefault("AiPlayerbot.TargetPosRecalcDistance", 0.1f);

    /* FORCE IT TO FALSE DUE TO ASYNC LOADING */
    BarGoLink::SetOutputState(false);
    //BarGoLink::SetOutputState(config.GetBoolDefault("AiPlayerbot.ShowProgressBars", false));
}
void PlayerbotAIConfig::loadTalent()
{
    for (uint32 cls = 1; cls < MAX_CLASSES; ++cls)
    {
        classSpecs[cls] = ClassSpecs(1 << (cls - 1));
        for (uint32 spec = 0; spec < MAX_LEVEL; ++spec)
        {
            ostringstream os; os << "AiPlayerbot.PremadeSpecName." << cls << "." << spec;
            string specName = config.GetStringDefault(os.str().c_str(), "");
            if (!specName.empty())
            {
                ostringstream os; os << "AiPlayerbot.PremadeSpecProb." << cls << "." << spec;
                int probability = config.GetIntDefault(os.str().c_str(), 100);

                TalentPath talentPath(spec, specName, probability);

                for (int level = 10; level <= 100; level++)
                {
                    ostringstream os; os << "AiPlayerbot.PremadeSpecLink." << cls << "." << spec << "." << level;
                    string specLink = config.GetStringDefault(os.str().c_str(), "");
                    specLink = specLink.substr(0, specLink.find("#", 0));;
                    specLink = specLink.substr(0, specLink.find(" ", 0));;

                    if (!specLink.empty())
                    {
                        ostringstream out;

                        //Ignore bad specs.
                        if (!classSpecs[cls].baseSpec.CheckTalentLink(specLink, &out))
                        {
                            sLog.outErrorDb("Error with premade spec link: %s", specLink.c_str());
                            sLog.outErrorDb("%s", out.str().c_str());
                            continue;
                        }

                        TalentSpec linkSpec(&classSpecs[cls].baseSpec, specLink);

                        if (!linkSpec.CheckTalents(level, &out))
                        {
                            sLog.outErrorDb("Error with premade spec: %s", specLink.c_str());
                            sLog.outErrorDb("%s", out.str().c_str());
                            continue;
                        }


                        talentPath.talentSpec.push_back(linkSpec);
                    }
                }

                //Only add paths that have atleast 1 spec.
                if (talentPath.talentSpec.size() > 0)
                    classSpecs[cls].talentPath.push_back(talentPath);
            }
        }
    }
}
void PlayerbotAIConfig::loadBotCheat()
{
    botCheats.clear();
    LoadListString<list<string>>(config.GetStringDefault("AiPlayerbot.BotCheats", "taxi"), botCheats);

    botCheatMask = 0;

    if (std::find(botCheats.begin(), botCheats.end(), "taxi") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::taxi;
    if (std::find(botCheats.begin(), botCheats.end(), "gold") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::gold;
    if (std::find(botCheats.begin(), botCheats.end(), "health") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::health;
    if (std::find(botCheats.begin(), botCheats.end(), "mana") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::mana;
    if (std::find(botCheats.begin(), botCheats.end(), "power") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::power;
}
void PlayerbotAIConfig::loadWorldBuff()
{
    LoadListString<list<string>>(config.GetStringDefault("AiPlayerbot.AllowedLogFiles", ""), allowedLogFiles);

    worldBuffs.clear();

    for (uint32 factionId = 0; factionId < 3; factionId++)
    {
        for (uint32 classId = 0; classId < MAX_CLASSES; classId++)
        {
            for (uint32 specId = 0; specId < 4; specId++)
            {
                for (uint32 minLevel = 0; minLevel < MAX_LEVEL; minLevel++)
                {
                    for (uint32 maxLevel = 0; maxLevel < MAX_LEVEL; maxLevel++)
                    {
                        loadWorldBuf(&config, factionId, classId, specId, minLevel, maxLevel);
                    }
                }
            }
        }
    }
}

bool PlayerbotAIConfig::Initialize()
{
    sLog.outString("Initializing AI Playerbot by ike3, based on the original Playerbot by blueboy");
    if (!config.SetSource(SYSCONFDIR"aiplayerbot.conf"))
    {
        sLog.outString("AI Playerbot is Disabled. Unable to open configuration file aiplayerbot.conf");
        return false;
    }
    enabled = config.GetBoolDefault("AiPlayerbot.Enabled", true);
    if (!enabled)
    {
        sLog.outString("AI Playerbot is Disabled in aiplayerbot.conf");
        return false;
    }

    loadConfigParameters();

    sLog.outString("---------------------------------------");
    sLog.outString("          Loading TalentSpecs          ");
    sLog.outString("---------------------------------------");
    sLog.outString();
    
    loadTalent();
    loadBotCheat();
    loadWorldBuff();

    RandomPlayerbotFactory::CreateRandomBots();
    PlayerbotFactory::Init();
    sRandomItemMgr.Init();

    auctionbot.Init();
    sRandomItemMgr.InitAfterAhBot();

    sPlayerbotTextMgr.LoadBotTexts();
    sPlayerbotTextMgr.LoadBotTextChance();

    if (sPlayerbotAIConfig.autoDoQuests)
    {
        sTravelMgr.LoadQuestTravelTable();
    }

    if (sPlayerbotAIConfig.randomBotJoinBG)
        sRandomPlayerbotMgr.LoadBattleMastersCache();

    sLog.outString("---------------------------------------");
    sLog.outString("        AI Playerbot initialized       ");
    sLog.outString("---------------------------------------");
    sLog.outString();

    return true;
}

bool PlayerbotAIConfig::IsInRandomAccountList(uint32 id)
{
    return find(randomBotAccounts.begin(), randomBotAccounts.end(), id) != randomBotAccounts.end();
}

bool PlayerbotAIConfig::IsInRandomQuestItemList(uint32 id)
{
    return find(randomBotQuestItems.begin(), randomBotQuestItems.end(), id) != randomBotQuestItems.end();
}

bool PlayerbotAIConfig::IsInPvpProhibitedZone(uint32 id)
{
	return find(pvpProhibitedZoneIds.begin(), pvpProhibitedZoneIds.end(), id) != pvpProhibitedZoneIds.end();
}

string PlayerbotAIConfig::GetValue(string name)
{
    ostringstream out;

    if (name == "GlobalCooldown")
        out << globalCoolDown;
    else if (name == "ReactDelay")
        out << reactDelay;

    else if (name == "SightDistance")
        out << sightDistance;
    else if (name == "SpellDistance")
        out << spellDistance;
    else if (name == "ReactDistance")
        out << reactDistance;
    else if (name == "GrindDistance")
        out << grindDistance;
    else if (name == "LootDistance")
        out << lootDistance;
    else if (name == "FleeDistance")
        out << fleeDistance;

    else if (name == "CriticalHealth")
        out << criticalHealth;
    else if (name == "LowHealth")
        out << lowHealth;
    else if (name == "MediumHealth")
        out << mediumHealth;
    else if (name == "AlmostFullHealth")
        out << almostFullHealth;
    else if (name == "LowMana")
        out << lowMana;

    else if (name == "IterationsPerTick")
        out << iterationsPerTick;

    return out.str();
}

void PlayerbotAIConfig::SetValue(string name, string value)
{
    istringstream out(value, istringstream::in);

    if (name == "GlobalCooldown")
        out >> globalCoolDown;
    else if (name == "ReactDelay")
        out >> reactDelay;

    else if (name == "SightDistance")
        out >> sightDistance;
    else if (name == "SpellDistance")
        out >> spellDistance;
    else if (name == "ReactDistance")
        out >> reactDistance;
    else if (name == "GrindDistance")
        out >> grindDistance;
    else if (name == "LootDistance")
        out >> lootDistance;
    else if (name == "FleeDistance")
        out >> fleeDistance;

    else if (name == "CriticalHealth")
        out >> criticalHealth;
    else if (name == "LowHealth")
        out >> lowHealth;
    else if (name == "MediumHealth")
        out >> mediumHealth;
    else if (name == "AlmostFullHealth")
        out >> almostFullHealth;
    else if (name == "LowMana")
        out >> lowMana;

    else if (name == "IterationsPerTick")
        out >> iterationsPerTick;
}


void PlayerbotAIConfig::loadWorldBuf(Config* config, uint32 factionId1, uint32 classId1, uint32 specId1, uint32 minLevel1, uint32 maxLevel1)
{
    list<uint32> buffs;

    ostringstream os; os << "AiPlayerbot.WorldBuff." << factionId1 << "." << classId1 << "." << specId1 << "." << minLevel1 << "." << maxLevel1;

    LoadList<list<uint32> >(config->GetStringDefault(os.str().c_str(), ""), buffs);

    for (auto buff : buffs)
    {
        worldBuff wb = { buff, factionId1, classId1, specId1, minLevel1, maxLevel1 };
        worldBuffs.push_back(wb);
    }

    if (maxLevel1 == 0)
    {
        ostringstream os; os << "AiPlayerbot.WorldBuff." << factionId1 << "." << classId1 << "." << specId1 << "." << minLevel1;

        LoadList<list<uint32> >(config->GetStringDefault(os.str().c_str(), ""), buffs);

        for (auto buff : buffs)
        {
            worldBuff wb = { buff, factionId1, classId1, specId1, minLevel1, maxLevel1 };
            worldBuffs.push_back(wb);
        }
    }

    if (maxLevel1 == 0 && minLevel1 == 0)
    {
        ostringstream os; os << "AiPlayerbot.WorldBuff." << factionId1 << "." << classId1 << "." << specId1;

        LoadList<list<uint32> >(config->GetStringDefault(os.str().c_str(), ""), buffs);

        for (auto buff : buffs)
        {
            worldBuff wb = { buff, factionId1, classId1, specId1, minLevel1, maxLevel1 };
            worldBuffs.push_back(wb);
        }
    }

    if (specId1 == 0 && maxLevel1 == 0 && minLevel1 == 0)
    {
        ostringstream os; os << "AiPlayerbot.WorldBuff." << factionId1 << "." << classId1;

        LoadList<list<uint32> >(config->GetStringDefault(os.str().c_str(), ""), buffs);

        for (auto buff : buffs)
        {
            worldBuff wb = { buff, factionId1, classId1, specId1, minLevel1, maxLevel1 };
            worldBuffs.push_back(wb);
        }
    }

    if (specId1 == 0 && classId1 == 0 && maxLevel1 == 0 && minLevel1 == 0)
    {
        ostringstream os; os << "AiPlayerbot.WorldBuff." << factionId1;

        LoadList<list<uint32> >(config->GetStringDefault(os.str().c_str(), ""), buffs);

        for (auto buff : buffs)
        {
            worldBuff wb = { buff, factionId1, classId1, specId1, minLevel1, maxLevel1 };
            worldBuffs.push_back(wb);
        }
    }

    if (factionId1 == 0 && specId1 == 0 && classId1 == 0 && maxLevel1 == 0 && minLevel1 == 0)
    {
        ostringstream os; os << "AiPlayerbot.WorldBuff";

        LoadList<list<uint32> >(config->GetStringDefault(os.str().c_str(), ""), buffs);

        for (auto buff : buffs)
        {
            worldBuff wb = { buff, factionId1, classId1, specId1, minLevel1, maxLevel1 };
            worldBuffs.push_back(wb);
        }
    }
}


std::string PlayerbotAIConfig::GetTimestampStr()
{
    time_t t = time(nullptr);
    tm* aTm = localtime(&t);
    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)
    //       HH     hour (2 digits 00-23)
    //       MM     minutes (2 digits 00-59)
    //       SS     seconds (2 digits 00-59)
    char buf[20];
    snprintf(buf, 20, "%04d-%02d-%02d %02d-%02d-%02d", aTm->tm_year + 1900, aTm->tm_mon + 1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
    return std::string(buf);
}

bool PlayerbotAIConfig::openLog(string fileName, char const* mode)
{
    if (!hasLog(fileName))
        return false;
     
    auto logFileIt = logFiles.find(fileName);
    if (logFileIt == logFiles.end())
    {
        logFiles.insert(make_pair(fileName, make_pair(nullptr, false)));
        logFileIt = logFiles.find(fileName);
    }

    FILE* file = logFileIt->second.first;
    bool fileOpen = logFileIt->second.second;

    if (fileOpen) //close log file
        fclose(file);

    string m_logsDir = sConfig.GetStringDefault("LogsDir");
    if (!m_logsDir.empty())
    {
        if ((m_logsDir.at(m_logsDir.length() - 1) != '/') && (m_logsDir.at(m_logsDir.length() - 1) != '\\'))
            m_logsDir.append("/");
    }


    file = fopen((m_logsDir + fileName).c_str(), mode);
    fileOpen = true;

    logFileIt->second.first = file;
    logFileIt->second.second = fileOpen;
    
    return true;
}

void PlayerbotAIConfig::log(string fileName, const char* str, ...)
{
    if (!str)
        return;

    std::lock_guard<std::mutex> guard(m_logMtx);

    if (!isLogOpen(fileName))
        if (!openLog(fileName, "a"))
            return;

    FILE* file = logFiles.find(fileName)->second.first;

    va_list ap;
    va_start(ap, str);
    vfprintf(file, str, ap);
    fprintf(file, "\n");
    va_end(ap);
    fflush(file);

    fflush(stdout);
}
