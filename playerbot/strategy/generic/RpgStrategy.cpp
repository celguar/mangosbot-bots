#include "botpch.h"
#include "../../playerbot.h"
#include "RpgStrategy.h"
#include "../actions/RpgSubActions.h"

using namespace ai;

float RpgActionMultiplier::GetValue(Action* action)
{
    if (action == NULL) return 1.0f;

    string nextAction = AI_VALUE(string, "next rpg action");
    string name = action->getName();

    if (!nextAction.empty() && dynamic_cast<RpgEnabled*>(action) && name != nextAction)
        return 0.0f;

    return 1.0f;
}

RpgStrategy::RpgStrategy(PlayerbotAI* ai) : Strategy(ai)
{
}

NextAction** RpgStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("rpg", 1.1f), NULL);
}

void RpgStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "no rpg target",
        NextAction::array(0, new NextAction("choose rpg target", 5.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "far from rpg target",
        NextAction::array(0, new NextAction("move to rpg target", 5.0f), NULL)));

    //Sub actions
    triggers.push_back(new TriggerNode(
        "rpg",
        NextAction::array(0, new NextAction("rpg stay", 1.001f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg",
        NextAction::array(0, new NextAction("rpg work", 1.001f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg",
        NextAction::array(0, new NextAction("rpg emote", 1.001f), NULL)));

    triggers.push_back(new TriggerNode(
        "has rpg target",
        NextAction::array(0, new NextAction("rpg cancel", 1.001f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg taxi",
        NextAction::array(0, new NextAction("rpg taxi", 1.005f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg discover",
        NextAction::array(0, new NextAction("rpg discover", 1.110f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg start quest",
        NextAction::array(0, new NextAction("rpg start quest", 1.080f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg end quest",
        NextAction::array(0, new NextAction("rpg end quest", 1.090f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg buy",
        NextAction::array(0, new NextAction("rpg buy", 1.030f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg sell",
        NextAction::array(0, new NextAction("rpg sell", 1.100f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg repair",
        NextAction::array(0, new NextAction("rpg repair", 1.095f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg train",
        NextAction::array(0, new NextAction("rpg train", 1.080f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg heal",
        NextAction::array(0, new NextAction("rpg heal", 1.025f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg home bind",
        NextAction::array(0, new NextAction("rpg home bind", 1.060f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg queue bg",
        NextAction::array(0, new NextAction("rpg queue bg", 1.085f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg buy petition",
        NextAction::array(0, new NextAction("rpg buy petition", 1.040f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg use",
        NextAction::array(0, new NextAction("rpg use", 1.001f), NULL)));

    /*triggers.push_back(new TriggerNode(
        "rpg spell",
        NextAction::array(0, new NextAction("rpg spell", 1.001f), NULL)));*/

    /*triggers.push_back(new TriggerNode(
        "rpg craft",
        NextAction::array(0, new NextAction("rpg craft", 1.001f), NULL)));*/

    triggers.push_back(new TriggerNode(
        "rpg trade useful",
        NextAction::array(0, new NextAction("rpg trade useful", 1.030f), NULL)));

    triggers.push_back(new TriggerNode(
        "rpg duel",
        NextAction::array(0, new NextAction("rpg duel", 1.010f), NULL)));
}

void RpgStrategy::InitMultipliers(std::list<Multiplier*>& multipliers)
{
    multipliers.push_back(new RpgActionMultiplier(ai));
}

