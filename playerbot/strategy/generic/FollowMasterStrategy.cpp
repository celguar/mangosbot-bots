#include "botpch.h"
#include "../../playerbot.h"
#include "FollowMasterStrategy.h"

using namespace ai;

void FollowMasterStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "out of react range",
        NextAction::array(0, new NextAction("flee to master", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "not near master",
        NextAction::array(0, new NextAction("follow", 1.0f), NULL)));
}
