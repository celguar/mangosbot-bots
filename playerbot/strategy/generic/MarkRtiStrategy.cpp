#include "botpch.h"
#include "../../playerbot.h"
#include "MarkRtiStrategy.h"

using namespace ai;

void MarkRtiStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "no rti target",
        NextAction::array(0, new NextAction("mark rti", ACTION_NORMAL), NULL)));
}

