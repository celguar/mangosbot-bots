#pragma once

#include "botpch.h"
#include "../../playerbot.h"
#include "LeaveGroupAction.h"


using namespace ai;

namespace ai
{
	bool LeaveGroupAction::Leave(Player* player) {

        if (!player)
            return false;

        Group* group = bot->GetGroup();

        /*if (!player->GetPlayerbotAI() && !ai->GetSecurity()->CheckLevelFor(PlayerbotSecurityLevel::PLAYERBOT_SECURITY_INVITE, false, player))
            return false;*/

        bool aiMaster = (ai->GetMaster() && ai->GetMaster()->GetPlayerbotAI());

        ai->TellMaster(BOT_TEXT("goodbye"), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_TALK, false);

        bool freeBot = sRandomPlayerbotMgr.IsFreeBot(bot);

        bool shouldStay = freeBot && bot->GetGroup() && player == bot;

        if (!shouldStay)
        {
            WorldPacket p;
            string member = bot->GetName();
            p << uint32(PARTY_OP_LEAVE) << member << uint32(0);
            bot->GetSession()->HandleGroupDisbandOpcode(p);
            if (ai->GetMaster() && ai->GetMaster()->GetObjectGuid() != player->GetObjectGuid())
                bot->Whisper("I left my group", LANG_UNIVERSAL, player->GetObjectGuid());
        }

        if (freeBot)
        {
            bot->GetPlayerbotAI()->SetMaster(NULL);
        }        

        if(!aiMaster)
            ai->ResetStrategies();
        ai->Reset();

        if(group)
            sPlayerbotAIConfig.logEvent(ai, "LeaveGroupAction", group->GetLeaderName(), to_string(group->GetMembersCount()));

        return true;
	}

    bool LeaveFarAwayAction::isUseful()
    {
        if (bot->InBattleGround())
            return false;

        if (bot->InBattleGroundQueue())
            return false;

        if (!bot->GetGroup())
            return false;      

        Player* groupMaster = ai->GetGroupMaster();

        if (!groupMaster)
            return false;

        if (ai->HasActivePlayerMaster())
            return false;

        for (GroupReference* gref = bot->GetGroup()->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->getSource();
            if (!ai->IsSafe(member))
                return false;
        }

        if (!sPlayerbotAIConfig.randomBotGroupNearby)
            return true;

        if (ai->GetGrouperType() == GrouperType::SOLO)
            return true;

        uint32 dCount = AI_VALUE(uint32, "death count");

        if (dCount > 9)
            return true;

        if (dCount > 4 && !ai->HasRealPlayerMaster())
            return true;

        if (bot->GetGuildId() == groupMaster->GetGuildId())
        {
            if (bot->GetLevel() > groupMaster->GetLevel() + 5)
            {
                if(AI_VALUE(bool, "should get money"))
                    return false;
            }
        }

        if (abs(int32(groupMaster->GetLevel() - bot->GetLevel())) > 4)
            return true;

        if (MEM_AI_VALUE(uint32, "experience")->LastChangeDelay() > 15 * MINUTE && MEM_AI_VALUE(uint32, "honor")->LastChangeDelay() > 15 * MINUTE)
            return true;

        return false;
    }
}