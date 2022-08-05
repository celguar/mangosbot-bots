#include "botpch.h"
#include "../../playerbot.h"
#include "TravelStrategy.h"

using namespace ai;

TravelStrategy::TravelStrategy(PlayerbotAI* ai) : Strategy(ai)
{
}

NextAction** TravelStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("travel", 1.0f), NULL);
}

void TravelStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        //"random",
        "no travel target",
        NextAction::array(0, new NextAction("choose travel target", 6.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "far from travel target",
        NextAction::array(0, new NextAction("move to travel target", 1), NULL)));
}
