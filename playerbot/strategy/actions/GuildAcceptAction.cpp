#include "botpch.h"
#include "../../playerbot.h"
#include "GuildAcceptAction.h"

using namespace std;
using namespace ai;

bool GuildAcceptAction::Execute(Event& event)
{
    WorldPacket p(event.getPacket());
    p.rpos(0);
    Player* inviter = nullptr;
    std::string Invitedname;
    p >> Invitedname;

    if (normalizePlayerName(Invitedname))
        inviter = ObjectAccessor::FindPlayerByName(Invitedname.c_str());

    if (!inviter)
        return false;

    bool accept = true;
    uint32 guildId = inviter->GetGuildId();
    if (!guildId)
    {
        ai->TellError("You are not in a guild!");
        accept = false;
    }
    else if (bot->GetGuildId())
    {
        ai->TellError("Sorry, I am in a guild already");
        accept = false;
    }
    else if (!ai->GetSecurity()->CheckLevelFor(PlayerbotSecurityLevel::PLAYERBOT_SECURITY_INVITE, false, inviter, true))
    {
        ai->TellError("Sorry, I don't want to join your guild :(");
        accept = false;
    }

    Guild* guild = sGuildMgr.GetGuildById(guildId);

    if(guild && guild->GetMemberSize() > 1000)
    {
        ai->TellError("This guild has over 1000 members. To stop it from reaching the 1064 member limit I will not join it.");
        accept = false;
    }

    WorldPacket packet;
    if (accept)
    {
        bot->GetSession()->HandleGuildAcceptOpcode(packet);
    }
    else
    {
        bot->GetSession()->HandleGuildDeclineOpcode(packet);
    }
    return true;
}