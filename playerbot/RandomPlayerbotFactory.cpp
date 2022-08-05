#include "Config/Config.h"
#include "../botpch.h"
#include "playerbot.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotFactory.h"
#include "AccountMgr.h"
#include "ObjectMgr.h"
#include "DatabaseEnv.h"
#include "PlayerbotAI.h"
#include "Player.h"
#include "RandomPlayerbotFactory.h"
#include "SystemConfig.h"
#include "Social/SocialMgr.h"

#ifndef MANGOSBOT_ZERO
#ifdef CMANGOS
#include "Arena/ArenaTeam.h"
#endif
#ifdef MANGOS
#include "ArenaTeam.h"
#endif
#endif


map<uint8, vector<uint8> > RandomPlayerbotFactory::availableRaces;

RandomPlayerbotFactory::RandomPlayerbotFactory(uint32 accountId) : accountId(accountId)
{
    availableRaces[CLASS_WARRIOR].push_back(RACE_HUMAN);
    availableRaces[CLASS_WARRIOR].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_WARRIOR].push_back(RACE_GNOME);
    availableRaces[CLASS_WARRIOR].push_back(RACE_DWARF);
    availableRaces[CLASS_WARRIOR].push_back(RACE_ORC);
    availableRaces[CLASS_WARRIOR].push_back(RACE_UNDEAD);
    availableRaces[CLASS_WARRIOR].push_back(RACE_TAUREN);
    availableRaces[CLASS_WARRIOR].push_back(RACE_TROLL);
#ifndef MANGOSBOT_ZERO
    availableRaces[CLASS_WARRIOR].push_back(RACE_DRAENEI);
#endif

    availableRaces[CLASS_PALADIN].push_back(RACE_HUMAN);
    availableRaces[CLASS_PALADIN].push_back(RACE_DWARF);
#ifndef MANGOSBOT_ZERO
    availableRaces[CLASS_PALADIN].push_back(RACE_DRAENEI);
    availableRaces[CLASS_PALADIN].push_back(RACE_BLOODELF);
#endif

    availableRaces[CLASS_ROGUE].push_back(RACE_HUMAN);
    availableRaces[CLASS_ROGUE].push_back(RACE_DWARF);
    availableRaces[CLASS_ROGUE].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_ROGUE].push_back(RACE_GNOME);
    availableRaces[CLASS_ROGUE].push_back(RACE_ORC);
    availableRaces[CLASS_ROGUE].push_back(RACE_TROLL);
#ifndef MANGOSBOT_ZERO
    availableRaces[CLASS_ROGUE].push_back(RACE_BLOODELF);
#endif

    availableRaces[CLASS_PRIEST].push_back(RACE_HUMAN);
    availableRaces[CLASS_PRIEST].push_back(RACE_DWARF);
    availableRaces[CLASS_PRIEST].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_PRIEST].push_back(RACE_TROLL);
    availableRaces[CLASS_PRIEST].push_back(RACE_UNDEAD);
#ifndef MANGOSBOT_ZERO
    availableRaces[CLASS_PRIEST].push_back(RACE_DRAENEI);
    availableRaces[CLASS_PRIEST].push_back(RACE_BLOODELF);
#endif

    availableRaces[CLASS_MAGE].push_back(RACE_HUMAN);
    availableRaces[CLASS_MAGE].push_back(RACE_GNOME);
    availableRaces[CLASS_MAGE].push_back(RACE_UNDEAD);
    availableRaces[CLASS_MAGE].push_back(RACE_TROLL);
#ifndef MANGOSBOT_ZERO
    availableRaces[CLASS_MAGE].push_back(RACE_DRAENEI);
    availableRaces[CLASS_MAGE].push_back(RACE_BLOODELF);
#endif

    availableRaces[CLASS_WARLOCK].push_back(RACE_HUMAN);
    availableRaces[CLASS_WARLOCK].push_back(RACE_GNOME);
    availableRaces[CLASS_WARLOCK].push_back(RACE_UNDEAD);
    availableRaces[CLASS_WARLOCK].push_back(RACE_ORC);
#ifndef MANGOSBOT_ZERO
    availableRaces[CLASS_WARLOCK].push_back(RACE_BLOODELF);
#endif

    availableRaces[CLASS_SHAMAN].push_back(RACE_ORC);
    availableRaces[CLASS_SHAMAN].push_back(RACE_TAUREN);
    availableRaces[CLASS_SHAMAN].push_back(RACE_TROLL);
#ifndef MANGOSBOT_ZERO
    availableRaces[CLASS_SHAMAN].push_back(RACE_DRAENEI);
#endif

    availableRaces[CLASS_HUNTER].push_back(RACE_DWARF);
    availableRaces[CLASS_HUNTER].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_HUNTER].push_back(RACE_ORC);
    availableRaces[CLASS_HUNTER].push_back(RACE_TAUREN);
    availableRaces[CLASS_HUNTER].push_back(RACE_TROLL);
#ifndef MANGOSBOT_ZERO
    availableRaces[CLASS_HUNTER].push_back(RACE_DRAENEI);
    availableRaces[CLASS_HUNTER].push_back(RACE_BLOODELF);
#endif

    availableRaces[CLASS_DRUID].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_DRUID].push_back(RACE_TAUREN);

#ifdef MANGOSBOT_TWO
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_TAUREN);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_HUMAN);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_ORC);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_UNDEAD);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_TROLL);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_BLOODELF);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_DRAENEI);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_GNOME);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_DWARF);
#endif
}

bool RandomPlayerbotFactory::CreateRandomBot(uint8 cls, unordered_map<uint8, vector<string>>& names)
{
    sLog.outDebug( "Creating new random bot for class %d", cls);

    uint8 gender = rand() % 2 ? GENDER_MALE : GENDER_FEMALE;

    uint8 race = availableRaces[cls][urand(0, availableRaces[cls].size() - 1)];

    string name;
    if(names.empty())
        name = CreateRandomBotName(gender);
    else
    {
        if (names[gender].empty())
            return false;
        uint32 i = urand(0, names[gender].size() - 1);
        name = names[gender][i];
        swap(names[gender][i], names[gender].back());
        names[gender].pop_back();
    }
    if (name.empty())
        return false;

    vector<uint8> skinColors, facialHairTypes;
    vector<pair<uint8,uint8>> faces, hairs;
    for (CharSectionsMap::const_iterator itr = sCharSectionMap.begin(); itr != sCharSectionMap.end(); ++itr)
    {
        CharSectionsEntry const* entry = itr->second;
        if (entry->Race != race || entry->Gender != gender)
            continue;

#ifndef MANGOSBOT_TWO
        switch (entry->BaseSection)
        {
        case SECTION_TYPE_SKIN:
            skinColors.push_back(entry->ColorIndex);
            break;
        case SECTION_TYPE_FACE:
            faces.push_back(pair<uint8,uint8>(entry->VariationIndex, entry->ColorIndex));
            break;
        case SECTION_TYPE_FACIAL_HAIR:
            facialHairTypes.push_back(entry->ColorIndex);
            break;
        case SECTION_TYPE_HAIR:
            hairs.push_back(pair<uint8,uint8>(entry->VariationIndex, entry->ColorIndex));
            break;
        }
#else
        switch (entry->BaseSection)
        {
        case SECTION_TYPE_SKIN:
            skinColors.push_back(entry->Color);
            break;
        case SECTION_TYPE_FACE:
            faces.push_back(pair<uint8, uint8>(entry->VariationIndex, entry->Color));
            break;
        case SECTION_TYPE_FACIAL_HAIR:
            facialHairTypes.push_back(entry->Color);
            break;
        case SECTION_TYPE_HAIR:
            hairs.push_back(pair<uint8, uint8>(entry->VariationIndex, entry->Color));
            break;
        }
#endif
    }

    uint8 skinColor = skinColors[urand(0, skinColors.size() - 1)];
    pair<uint8,uint8> face = faces[urand(0, faces.size() - 1)];
    pair<uint8,uint8> hair = hairs[urand(0, hairs.size() - 1)];

	bool excludeCheck = (race == RACE_TAUREN) || (gender == GENDER_FEMALE && race != RACE_NIGHTELF && race != RACE_UNDEAD);
#ifndef MANGOSBOT_TWO
	uint8 facialHair = excludeCheck ? 0 : facialHairTypes[urand(0, facialHairTypes.size() - 1)];
#else
	uint8 facialHair = 0;
#endif
	//TODO vector crash on cmangos TWO when creating one of the first bot characters, need a fix

	WorldSession* session = new WorldSession(accountId, NULL, SEC_PLAYER,

#ifdef MANGOSBOT_TWO
        2, 0, LOCALE_enUS, "", 0, 0, false);
#endif
#ifdef MANGOSBOT_ONE
		2, 0, LOCALE_enUS, "", 0, 0, false);
#endif
#ifdef MANGOSBOT_ZERO
        0, LOCALE_enUS, "", 0);
#endif

    session->SetNoAnticheat();

    Player* player = new Player(session);
    if (!player || !session)
    {
        sLog.outError("BOTS: Unable to create session or player for random acc %d - name: \"%s\"; race: %u; class: %u", accountId, name.c_str(), race, cls);
        return false;
    }
	if (!player->Create(sObjectMgr.GeneratePlayerLowGuid(), name, race, cls, gender,
	        face.second, // skinColor,
	        face.first,
	        hair.first,
	        hair.second, // hairColor,
	        facialHair, 0))
    {
        player->DeleteFromDB(player->GetObjectGuid(), accountId, true, true);
        delete session;
        delete player;
        sLog.outError("Unable to create random bot for account %d - name: \"%s\"; race: %u; class: %u",
                accountId, name.c_str(), race, cls);
        return false;
    }

    player->setCinematic(2);
    player->SetAtLoginFlag(AT_LOGIN_NONE);
    //player->SetSemaphoreTeleportFar(true); //Fake teleport to delay sql save
    //player->SaveToDB();
    //player->SetSemaphoreTeleportFar(false);

    sObjectAccessor.AddObject(player);

    sLog.outDebug( "Random bot created for account %d - name: \"%s\"; race: %u; class: %u",
            accountId, name.c_str(), race, cls);

    return true;
}

string RandomPlayerbotFactory::CreateRandomBotName(uint8 gender)
{
    QueryResult* result = CharacterDatabase.Query("SELECT MAX(name_id) FROM ai_playerbot_names");
    if (!result)
    {
        sLog.outError("No more names left for random bots");
        return "";
    }

    Field *fields = result->Fetch();
    uint32 maxId = fields[0].GetUInt32();
    delete result;

    result = CharacterDatabase.PQuery("SELECT n.name FROM ai_playerbot_names n LEFT OUTER JOIN characters e ON e.name = n.name WHERE e.guid IS NULL and n.gender = '%u' order by rand() limit 1", gender);
    if (!result)
    {
        sLog.outError("No more names left for random bots");
        return "";
    }

	fields = result->Fetch();
    string bname = fields[0].GetString();
    delete result;
    return bname;
}


void RandomPlayerbotFactory::CreateRandomBots()
{
    // check if scheduled for delete
    bool delAccs = false;
    bool delFriends = false;
    QueryResult* results = PlayerbotDatabase.Query(
        "select value from ai_playerbot_random_bots where event = 'bot_delete'");

    if (results)
    {
        delAccs = true;

        Field* fields = results->Fetch();
        uint32 deleteType = fields[0].GetUInt32();

        if (deleteType > 1)
            delFriends = true;

        PlayerbotDatabase.PExecute("DELETE FROM ai_playerbot_random_bots where event = 'bot_delete'");
        delete results;
    }

    if (sPlayerbotAIConfig.deleteRandomBotAccounts || delAccs)
    {
        std::list<uint32> botAccounts;
        std::list<uint32> botFriends;

        for (uint32 accountNumber = 0; accountNumber < sPlayerbotAIConfig.randomBotAccountCount; ++accountNumber)
        {
            ostringstream out; out << sPlayerbotAIConfig.randomBotAccountPrefix << accountNumber;
            string accountName = out.str();

            QueryResult* results = LoginDatabase.PQuery("SELECT id FROM account where username = '%s'", accountName.c_str());
            if (!results)
                continue;

            Field* fields = results->Fetch();
            uint32 accountId = fields[0].GetUInt32();
            delete results;

            botAccounts.push_back(accountId);
        }

        if (!delFriends)
            sLog.outString("Deleting random bot characters without friends/guild...");
        else
            sLog.outString("Deleting all random bot characters...");

#ifdef MANGOSBOT_TWO
        BarGoLink bar(sPlayerbotAIConfig.randomBotAccountCount * 10);
#else
        BarGoLink bar(sPlayerbotAIConfig.randomBotAccountCount * 9);
#endif

        // load list of friends
        if (!delFriends)
        {
            QueryResult* result = CharacterDatabase.PQuery("SELECT friend FROM character_social WHERE flags='%u'", SOCIAL_FLAG_FRIEND);
            if (result)
            {
                do
                {
                    Field* fields = result->Fetch();
                    uint32 guidlo = fields[0].GetUInt32();
                    botFriends.push_back(guidlo);

                } while (result->NextRow());

                delete result;
            }
        }

        vector<std::future<void>> dels;

        QueryResult* results = LoginDatabase.PQuery("SELECT id FROM account where username like '%s%%'", sPlayerbotAIConfig.randomBotAccountPrefix.c_str());
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                uint32 accId = fields[0].GetUInt32();

                if (!delFriends)
                {
                    // existing characters list
                    QueryResult* result = CharacterDatabase.PQuery("SELECT guid FROM characters WHERE account='%u'", accId);
                    if (result)
                    {
                        do
                        {
                            Field* fields = result->Fetch();
                            uint32 guidlo = fields[0].GetUInt32();
                            ObjectGuid guid = ObjectGuid(HIGHGUID_PLAYER, guidlo);

                            // if bot is someone's friend - don't delete it
                            if ((find(botFriends.begin(), botFriends.end(), guidlo) != botFriends.end()) && !delFriends)
                                continue;

                            // if bot is in someone's guild - don't delete it
                            uint32 guildId = Player::GetGuildIdFromDB(guid);
                            if (guildId && !delFriends)
                            {
                                Guild* guild = sGuildMgr.GetGuildById(guildId);
                                uint32 accountId = sObjectMgr.GetPlayerAccountIdByGUID(guild->GetLeaderGuid());

                                if (find(botAccounts.begin(), botAccounts.end(), accountId) == botAccounts.end())
                                    continue;
                            }

                            Player::DeleteFromDB(guid, accId, false, true);       // no need to update realm characters
                            bar.step();

                        } while (result->NextRow());

                        delete result;
                    }
                }
                else
                {
                    bar.step();
                    sAccountMgr.DeleteAccount(accId);
                    //dels.push_back(std::async([accId] {sAccountMgr.DeleteAccount(accId); }));
                }

            } while (results->NextRow());
			delete results;
        }

        /*bargolink bar4(dels .size());
        for (uint32 i=0;i<dels.size();i++)
        {
            bar4.step();
            dels[i].wait();
        }*/

        PlayerbotDatabase.Execute("DELETE FROM ai_playerbot_random_bots");
        sLog.outString("Random bot characters deleted");
    }
	int totalAccCount = sPlayerbotAIConfig.randomBotAccountCount;
	sLog.outString("Creating random bot accounts...");
    
    vector<std::future<void>> account_creations;

    BarGoLink bar(totalAccCount);
    for (uint32 accountNumber = 0; accountNumber < sPlayerbotAIConfig.randomBotAccountCount; ++accountNumber)
    {
        ostringstream out; out << sPlayerbotAIConfig.randomBotAccountPrefix << accountNumber;
        string accountName = out.str();
        QueryResult* results = LoginDatabase.PQuery("SELECT id FROM account where username = '%s'", accountName.c_str());
        if (results)
        {
            delete results;
            continue;
        }

        string password = "";
        if (sPlayerbotAIConfig.randomBotRandomPassword)
        {
            for (int i = 0; i < 10; i++)
            {
                password += (char)urand('!', 'z');
            }
        }
        else
            password = accountName;

#ifndef MANGOSBOT_ZERO
        uint8 max_expansion = MAX_EXPANSION;
        account_creations.push_back(std::async([accountName, password, max_expansion] {sAccountMgr.CreateAccount(accountName, password, max_expansion); }));
#else
        account_creations.push_back(std::async([accountName, password] {sAccountMgr.CreateAccount(accountName, password); }));
#endif

        sLog.outDebug("Account %s created for random bots", accountName.c_str());
        bar.step();
    }

    BarGoLink bar3(account_creations.size());
    for (uint32 i = 0; i < account_creations.size(); i++)
    {
        bar3.step();
        account_creations[i].wait();
    }

    //LoginDatabase.PExecute("UPDATE account SET expansion = '%u' where username like '%s%%'", 2, sPlayerbotAIConfig.randomBotAccountPrefix.c_str());

    int totalRandomBotChars = 0;
	int totalCharCount = sPlayerbotAIConfig.randomBotAccountCount
#ifdef MANGOSBOT_TWO
		* 10;
#else
		* 9;
#endif

    sLog.outString("Loading available names...");
    
    unordered_map<uint8,vector<string>> names;
    QueryResult* result = CharacterDatabase.PQuery("SELECT n.gender, n.name FROM ai_playerbot_names n LEFT OUTER JOIN characters e ON e.name = n.name WHERE e.guid IS NULL");
    if (!result)
    {
        sLog.outError("No more names left for random bots");
        return;
    }

    do
    {
        Field* fields = result->Fetch();
        uint8 gender = fields[0].GetUInt8();
        string bname = fields[1].GetString();
        names[gender].push_back(bname);
    } while (result->NextRow());

    delete result;

	sLog.outString("Creating random bot characters...");
	BarGoLink bar1(totalCharCount);
    for (uint32 accountNumber = 0; accountNumber < sPlayerbotAIConfig.randomBotAccountCount; ++accountNumber)
    {
        ostringstream out; out << sPlayerbotAIConfig.randomBotAccountPrefix << accountNumber;
        string accountName = out.str();

        QueryResult* results = LoginDatabase.PQuery("SELECT id FROM account where username = '%s'", accountName.c_str());
        if (!results)
            continue;

        Field* fields = results->Fetch();
        uint32 accountId = fields[0].GetUInt32();
		delete results;

        sPlayerbotAIConfig.randomBotAccounts.push_back(accountId);

        int count = sAccountMgr.GetCharactersCount(accountId);
#ifdef MANGOSBOT_TWO
        if (count >= 10)
#else
		if (count >= 9)
#endif
        {
            totalRandomBotChars += count;
            continue;
        }

        RandomPlayerbotFactory factory(accountId);
        for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES - count; ++cls)
        {
            // skip nonexistent classes
            if (!((1 << (cls - 1)) & CLASSMASK_ALL_PLAYABLE) || !sChrClassesStore.LookupEntry(cls))
                continue;

#ifdef MANGOSBOT_TWO
            if (cls != 10)
#else
            if (cls != 10 && cls != 6)
#endif
			{
                factory.CreateRandomBot(cls, names);
				bar1.step();
			}
        }

        totalRandomBotChars += sAccountMgr.GetCharactersCount(accountId);
    }

    vector<std::future<void>> bot_creations;

    BarGoLink bar2(sObjectAccessor.GetPlayers().size());
    for (auto pl : sObjectAccessor.GetPlayers())
    {
        Player* player = pl.second;
        account_creations.push_back(std::async([player] {player->SaveToDB(); }));
    }

    for (uint32 i = 0; i < account_creations.size(); i++)
    {
        bar2.step();
        account_creations[i].wait();
    }    

    sLog.outString("%zu random bot accounts with %d characters available", sPlayerbotAIConfig.randomBotAccounts.size(), totalRandomBotChars);
}


void RandomPlayerbotFactory::CreateRandomGuilds()
{
    vector<uint32> randomBots;

    QueryResult* results = PlayerbotDatabase.PQuery(
            "select `bot` from ai_playerbot_random_bots where event = 'add'");

    if (results)
    {
        do
        {
            Field* fields = results->Fetch();
            uint32 bot = fields[0].GetUInt32();
            randomBots.push_back(bot);
        } while (results->NextRow());
        delete results;
    }

    if (sPlayerbotAIConfig.deleteRandomBotGuilds)
    {
        sLog.outString("Deleting random bot guilds...");
        for (vector<uint32>::iterator i = randomBots.begin(); i != randomBots.end(); ++i)
        {
            ObjectGuid leader(HIGHGUID_PLAYER, *i);
            Guild* guild = sGuildMgr.GetGuildByLeader(leader);
            if (guild) guild->Disband();
        }
        sLog.outString("Random bot guilds deleted");
    }

    uint32 guildNumber = 0;
    vector<ObjectGuid> availableLeaders;
    for (vector<uint32>::iterator i = randomBots.begin(); i != randomBots.end(); ++i)
    {
        ObjectGuid leader(HIGHGUID_PLAYER, *i);
        Guild* guild = sGuildMgr.GetGuildByLeader(leader);
        if (guild)
        {
            ++guildNumber;
            sPlayerbotAIConfig.randomBotGuilds.push_back(guild->GetId());
        }
        else
        {
            Player* player = sObjectMgr.GetPlayer(leader);
            if (player && !player->GetGuildId())
                availableLeaders.push_back(leader);
        }
    }

    for (; guildNumber < sPlayerbotAIConfig.randomBotGuildCount; ++guildNumber)
    {
        string guildName = CreateRandomGuildName();
        if (guildName.empty())
            continue;

        if (availableLeaders.empty())
        {
            sLog.outError("No leaders for random guilds available");
			continue;
        }

        int index = urand(0, availableLeaders.size() - 1);
        ObjectGuid leader = availableLeaders[index];
        Player* player = sObjectMgr.GetPlayer(leader);
        if (!player)
        {
            sLog.outError("Cannot find player for leader %s", player->GetName());
			continue;
        }

        Guild* guild = new Guild();
        if (!guild->Create(player, guildName))
        {
            sLog.outError("Error creating guild %s", guildName.c_str());
			continue;
        }

        sGuildMgr.AddGuild(guild);

        // create random emblem
        uint32 st, cl, br, bc, bg;
        bg = urand(0, 51);
        bc = urand(0, 17);
        cl = urand(0, 17);
        br = urand(0, 7);
        st = urand(0, 180);
        guild->SetEmblem(st, cl, br, bc, bg);

        sPlayerbotAIConfig.randomBotGuilds.push_back(guild->GetId());
    }

    sLog.outString("%d random bot guilds available", guildNumber);
}

string RandomPlayerbotFactory::CreateRandomGuildName()
{
    QueryResult* result = CharacterDatabase.Query("SELECT MAX(name_id) FROM ai_playerbot_guild_names");
    if (!result)
    {
        sLog.outError("No more names left for random guilds");
        return "";
    }

    Field *fields = result->Fetch();
    uint32 maxId = fields[0].GetUInt32();
    delete result;

    uint32 id = urand(0, maxId);
    result = CharacterDatabase.PQuery("SELECT n.name FROM ai_playerbot_guild_names n "
            "LEFT OUTER JOIN guild e ON e.name = n.name "
            "WHERE e.guildid IS NULL AND n.name_id >= '%u' LIMIT 1", id);
    if (!result)
    {
        sLog.outError("No more names left for random guilds");
        return "";
    }

    fields = result->Fetch();
    string gname = fields[0].GetString();
    delete result;
    return gname;
}

#ifndef MANGOSBOT_ZERO
void RandomPlayerbotFactory::CreateRandomArenaTeams()
{
    vector<uint32> randomBots;

    QueryResult* results = PlayerbotDatabase.PQuery(
        "select `bot` from ai_playerbot_random_bots where event = 'add'");

    if (results)
    {
        do
        {
            Field* fields = results->Fetch();
            uint32 bot = fields[0].GetUInt32();
            randomBots.push_back(bot);
        } while (results->NextRow());
        delete results;
    }

    if (sPlayerbotAIConfig.deleteRandomBotArenaTeams)
    {
        sLog.outString("Deleting random bot arena teams...");
        for (vector<uint32>::iterator i = randomBots.begin(); i != randomBots.end(); ++i)
        {
            ObjectGuid captain(HIGHGUID_PLAYER, *i);
            ArenaTeam* arenateam = sObjectMgr.GetArenaTeamByCaptain(captain);
            if (arenateam)
                //sObjectMgr.RemoveArenaTeam(arenateam->GetId());
                arenateam->Disband(NULL);
        }
        sLog.outString("Random bot arena teams deleted");
    }

    uint32 arenaTeamNumber = 0;
    vector<ObjectGuid> availableCaptains;
    for (vector<uint32>::iterator i = randomBots.begin(); i != randomBots.end(); ++i)
    {
        ObjectGuid captain(HIGHGUID_PLAYER, *i);
        ArenaTeam* arenateam = sObjectMgr.GetArenaTeamByCaptain(captain);
        if (arenateam)
        {
            ++arenaTeamNumber;
            sPlayerbotAIConfig.randomBotArenaTeams.push_back(arenateam->GetId());
        }
        else
        {
            Player* player = sObjectMgr.GetPlayer(captain);

            if (!arenateam && player && player->GetLevel() >= 70)
                availableCaptains.push_back(captain);
        }
    }

    for (; arenaTeamNumber < sPlayerbotAIConfig.randomBotArenaTeamCount; ++arenaTeamNumber)
    {
        string arenaTeamName = CreateRandomArenaTeamName();
        if (arenaTeamName.empty())
            continue;

        if (availableCaptains.empty())
        {
            sLog.outError("No captains for random arena teams available");
            continue;
        }

        int index = urand(0, availableCaptains.size() - 1);
        ObjectGuid captain = availableCaptains[index];
        Player* player = sObjectMgr.GetPlayer(captain);
        if (!player)
        {
            sLog.outError("Cannot find player for captain %d", captain);
            continue;
        }

        if (player->GetLevel() < 70)
        {
            sLog.outError("Bot %d must be level 70 to create an arena team", captain);
            continue;
        }

        QueryResult* results = CharacterDatabase.PQuery("SELECT `type` FROM ai_playerbot_arena_team_names WHERE name = '%s'", arenaTeamName.c_str());
        if (!results)
        {
            sLog.outError("No valid types for arena teams");
            return;
        }

        Field *fields = results->Fetch();
        uint8 slot = fields[0].GetUInt32();
        delete results;

        ArenaType type;
        switch (slot)
        {
        case 2:
            type = ARENA_TYPE_2v2;
            break;
        case 3:
            type = ARENA_TYPE_3v3;
            break;
        case 5:
            type = ARENA_TYPE_5v5;
            break;
        }

        ArenaTeam* arenateam = new ArenaTeam();
        if (!arenateam->Create(player->GetObjectGuid(), type, arenaTeamName))
        {
            sLog.outError("Error creating arena team %s", arenaTeamName.c_str());
            continue;
        }
        arenateam->SetCaptain(player->GetObjectGuid());
        // set random rating
        arenateam->SetRatingForAll(urand(1500, 2700));
        // set random emblem
        uint32 backgroundColor = urand(0xFF000000, 0xFFFFFFFF), emblemStyle = urand(0, 101), emblemColor = urand(0xFF000000, 0xFFFFFFFF), borderStyle = urand(0, 5), borderColor = urand(0xFF000000, 0xFFFFFFFF);
        arenateam->SetEmblem(backgroundColor, emblemStyle, emblemColor, borderStyle, borderColor);
        // set random kills (wip)
        //arenateam->SetStats(STAT_TYPE_GAMES_WEEK, urand(0, 30));
        //arenateam->SetStats(STAT_TYPE_WINS_WEEK, urand(0, arenateam->GetStats().games_week));
        //arenateam->SetStats(STAT_TYPE_GAMES_SEASON, urand(arenateam->GetStats().games_week, arenateam->GetStats().games_week * 5));
        //arenateam->SetStats(STAT_TYPE_WINS_SEASON, urand(arenateam->GetStats().wins_week, arenateam->GetStats().games_season));
        sObjectMgr.AddArenaTeam(arenateam);
        sPlayerbotAIConfig.randomBotArenaTeams.push_back(arenateam->GetId());
        arenateam->SaveToDB();
    }

    sLog.outString("%d random bot arena teams available", arenaTeamNumber);
}

string RandomPlayerbotFactory::CreateRandomArenaTeamName()
{
    QueryResult* result = CharacterDatabase.Query("SELECT MAX(name_id) FROM ai_playerbot_arena_team_names");
    if (!result)
    {
        sLog.outError("No more names left for random arena teams");
        return "";
    }

    Field *fields = result->Fetch();
    uint32 maxId = fields[0].GetUInt32();
    delete result;

    uint32 id = urand(0, maxId);
    result = CharacterDatabase.PQuery("SELECT n.name FROM ai_playerbot_arena_team_names n "
        "LEFT OUTER JOIN arena_team e ON e.name = n.name "
        "WHERE e.arenateamid IS NULL AND n.name_id >= '%u' LIMIT 1", id);
    if (!result)
    {
        sLog.outError("No more names left for random arena teams");
        return "";
    }

    fields = result->Fetch();
    string aname = fields[0].GetString();
    delete result;
    return aname;
}
#endif

