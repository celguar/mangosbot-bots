#pragma once

#include "../Action.h"

namespace ai
{
    class AcceptInvitationAction : public Action {
    public:
        AcceptInvitationAction(PlayerbotAI* ai) : Action(ai, "accept invitation") {}

        virtual bool Execute(Event& event)
        {
            Group* grp = bot->GetGroupInvite();
            if (!grp)
                return false;

            Player* inviter = sObjectMgr.GetPlayer(grp->GetLeaderGuid());
            if (!inviter)
                return false;

			if (!ai->GetSecurity()->CheckLevelFor(PlayerbotSecurityLevel::PLAYERBOT_SECURITY_INVITE, false, inviter))
            {
                WorldPacket data(SMSG_GROUP_DECLINE, 10);
                data << bot->GetName();
                sServerFacade.SendPacket(inviter, data);
                bot->UninviteFromGroup();
                return false;
            }
            
            if (bot->isAFK())
                bot->ToggleAFK();

            WorldPacket p;
            uint32 roles_mask = 0;
            p << roles_mask;
            bot->GetSession()->HandleGroupAcceptOpcode(p);

            if (!bot->GetGroup() || !bot->GetGroup()->IsMember(inviter->GetObjectGuid()))
                return false;

            if (sRandomPlayerbotMgr.IsFreeBot(bot))
                ai->SetMaster(inviter);

            Player* master = inviter;

            ai->ResetStrategies();
            ai->ChangeStrategy("+follow,-lfg,-bg", BotState::BOT_STATE_NON_COMBAT);
            ai->Reset();

            ai->TellMaster(BOT_TEXT("hello"), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

            sPlayerbotAIConfig.logEvent(ai, "AcceptInvitationAction", grp->GetLeaderName(), to_string(grp->GetMembersCount()));

            if (master->GetPlayerbotAI()) //Copy formation from bot master.
            {
                Formation* masterFormation = MAI_VALUE(Formation*, "formation");
                FormationValue* value = (FormationValue*)context->GetValue<Formation*>("formation");
                value->Load(masterFormation->getName());
            }

            return true;
        }

        bool isUsefulWhenStunned() override { return true; }
    };

}
