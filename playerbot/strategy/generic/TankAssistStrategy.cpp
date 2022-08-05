#include "botpch.h"
#include "../../playerbot.h"
#include "TankAssistStrategy.h"

using namespace ai;

void TankAssistStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "tank assist",
        NextAction::array(0, new NextAction("tank assist", 60.0f), NULL)));
}
