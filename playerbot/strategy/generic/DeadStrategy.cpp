#include "botpch.h"
#include "../../playerbot.h"
#include "../Strategy.h"
#include "DeadStrategy.h"

using namespace ai;

void DeadStrategy::InitTriggers(std::vector<TriggerNode*> & triggers)
{
    PassTroughStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("auto release", relevance), NULL)));

    triggers.push_back(new TriggerNode(
        "dead",
       NextAction::array(0, new NextAction("find corpse", relevance), NULL)));

    triggers.push_back(new TriggerNode(
        "corpse near",
        NextAction::array(0, new NextAction("revive from corpse", relevance-1.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "resurrect request",
        NextAction::array(0, new NextAction("accept resurrect", relevance), NULL)));

    triggers.push_back(new TriggerNode(
        "falling far",
        NextAction::array(0, new NextAction("repop", relevance+1), NULL)));

    triggers.push_back(new TriggerNode(
        "location stuck",
        NextAction::array(0, new NextAction("repop", relevance+1), NULL)));

}

DeadStrategy::DeadStrategy(PlayerbotAI* ai) : PassTroughStrategy(ai)
{
}
