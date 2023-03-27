#include "botpch.h"
#include "../../playerbot.h"
#include "ChatShortcutActions.h"
#include "../../PlayerbotAIConfig.h"
#include "../values/PositionValue.h"
#include "../values/Formations.h"

using namespace ai;

void ReturnPositionResetAction::ResetPosition(string posName)
{
    ai::PositionMap& posMap = context->GetValue<ai::PositionMap&>("position")->Get();
    ai::PositionEntry pos = posMap[posName];
    pos.Reset();
    posMap[posName] = pos;
}

void ReturnPositionResetAction::SetPosition(WorldPosition wPos, string posName)
{
    ai::PositionMap& posMap = context->GetValue<ai::PositionMap&>("position")->Get();
    ai::PositionEntry pos = posMap[posName];
    pos.Set(wPos);
    posMap[posName] = pos;
}

void ReturnPositionResetAction::PrintStrategies(PlayerbotAI* ai, Event& event)
{
    if (event.getParam() == "?")
    {
        ai->ChangeStrategy("?", BotState::BOT_STATE_NON_COMBAT);
        ai->ChangeStrategy("?", BotState::BOT_STATE_COMBAT);
        ai->ChangeStrategy("?", BotState::BOT_STATE_REACTION);
    }
}

bool FollowChatShortcutAction::Execute(Event& event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    ai->Reset();
    ai->ChangeStrategy("+follow,-passive,-stay", BotState::BOT_STATE_NON_COMBAT);
    ai->ChangeStrategy("-stay,-guard", BotState::BOT_STATE_COMBAT);

    if(ai->HasStrategy("passive", BotState::BOT_STATE_COMBAT)) //Remove flee
        ai->ChangeStrategy("-passive,-follow", BotState::BOT_STATE_COMBAT);

    ai::PositionMap& posMap = context->GetValue<ai::PositionMap&>("position")->Get();
    ai::PositionEntry pos = posMap["return"];
    pos.Reset();
    posMap["return"] = pos;

    ReturnPositionResetAction::PrintStrategies(ai, event);

    Formation* formation = AI_VALUE(Formation*, "formation");
    MEM_AI_VALUE(WorldPosition, "master position")->Reset();

    if (formation->getName() == "custom") //If in custom formation set relative position to current position.
    {
        ai::PositionEntry pos = posMap["follow"];

        WorldPosition relPos(bot);

        if (!ai->IsSafe(master) || sServerFacade.GetDistance2d(bot, master) > sPlayerbotAIConfig.reactDistance) //Use default formation location.
        {
            relPos = WorldPosition(bot->GetMapId(), cos(GetFollowAngle()) * ai->GetRange("follow"), sin(GetFollowAngle()) * ai->GetRange("follow"), 0);
        }
        else //Use relative location.
        {
            relPos -= WorldPosition(ai->GetMaster());
            relPos.rotateXY(-1 * ai->GetMaster()->GetOrientation());
        }

        pos.Set(relPos.getX(), relPos.getY(), relPos.getZ(), relPos.getMapId());
        posMap["follow"] = pos;
    }

    if (sServerFacade.IsInCombat(bot))
    {     
        WorldLocation loc = formation->GetLocation();
        if (Formation::IsNullLocation(loc) || loc.mapid == -1)
            return false;

        if (MoveTo(loc.mapid, loc.coord_x, loc.coord_y, loc.coord_z, false, false))
        {
            ai->TellError(BOT_TEXT("following"));
            return true;
        }
    }

    ai->TellError(BOT_TEXT("following"));
    return true;
}

bool StayChatShortcutAction::Execute(Event& event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    ai->Reset();
    ai->ChangeStrategy("+stay,-follow,-passive", BotState::BOT_STATE_NON_COMBAT);
    ai->ChangeStrategy("+stay,-follow,-passive", BotState::BOT_STATE_COMBAT);

    SetPosition(bot);
    SetPosition(bot, "stay");
    MEM_AI_VALUE(WorldPosition, "master position")->Reset();

    PrintStrategies(ai, event);

    ai->TellError(BOT_TEXT("staying"));
    return true;
}

bool GuardChatShortcutAction::Execute(Event& event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    ai->Reset();
    ai->ChangeStrategy("+guard,-follow,-passive", BotState::BOT_STATE_NON_COMBAT);
    ai->ChangeStrategy("+guard,-follow,-passive", BotState::BOT_STATE_COMBAT);

    SetPosition(bot);
    SetPosition(bot, "guard");
    MEM_AI_VALUE(WorldPosition, "master position")->Reset();  

    PrintStrategies(ai, event);

    ai->TellError(BOT_TEXT("guarding"));
    return true;
}

bool FreeChatShortcutAction::Execute(Event& event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    ai->Reset();
    ai->ChangeStrategy("+free,-passive", BotState::BOT_STATE_NON_COMBAT);
    ai->ChangeStrategy("+free,-passive", BotState::BOT_STATE_COMBAT);

    PrintStrategies(ai, event);

    ai->TellError(BOT_TEXT("free_moving"));
    return true;
}

bool FleeChatShortcutAction::Execute(Event& event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    ai->Reset();
    ai->ChangeStrategy("+follow,+passive", BotState::BOT_STATE_NON_COMBAT);
    ai->ChangeStrategy("+follow,+passive", BotState::BOT_STATE_COMBAT);
    ResetPosition();

    PrintStrategies(ai, event);

    if (bot->GetMapId() != master->GetMapId() || sServerFacade.GetDistance2d(bot, master) > sPlayerbotAIConfig.sightDistance)
    {
        ai->TellError(BOT_TEXT("fleeing_far"));
        return true;
    }
    ai->TellError(BOT_TEXT("fleeing"));
    return true;
}

bool GoawayChatShortcutAction::Execute(Event& event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    ai->Reset();
    ai->ChangeStrategy("+runaway", BotState::BOT_STATE_NON_COMBAT);
    ai->ChangeStrategy("+runaway", BotState::BOT_STATE_COMBAT);
    ResetPosition();

    PrintStrategies(ai, event);

    ai->TellError("Running away");
    return true;
}

bool GrindChatShortcutAction::Execute(Event& event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    ai->Reset();
    ai->ChangeStrategy("+grind,-passive", BotState::BOT_STATE_NON_COMBAT);
    ResetPosition();
    ai->TellError(BOT_TEXT("grinding"));
    return true;
}

bool TankAttackChatShortcutAction::Execute(Event& event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    if (!ai->IsTank(bot))
        return false;

    ai->Reset();
    ai->ChangeStrategy("-passive", BotState::BOT_STATE_NON_COMBAT);
    ai->ChangeStrategy("-passive", BotState::BOT_STATE_COMBAT);
    ResetPosition();
    ai->TellError(BOT_TEXT("attacking"));
    return true;
}

bool MaxDpsChatShortcutAction::Execute(Event& event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    if (!ai->ContainsStrategy(STRATEGY_TYPE_DPS))
        return false;

    ai->Reset();
    ai->ChangeStrategy("-threat,-conserve mana,-cast time,+dps debuff,+boost", BotState::BOT_STATE_COMBAT);
    ai->TellError("Max DPS!");
    return true;
}
