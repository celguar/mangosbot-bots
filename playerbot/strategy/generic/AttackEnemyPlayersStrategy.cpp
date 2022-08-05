#include "botpch.h"
#include "../../playerbot.h"
#include "AttackEnemyPlayersStrategy.h"

using namespace ai;

void AttackEnemyPlayersStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "enemy player near",
        NextAction::array(0, new NextAction("attack enemy player", 69.0f), NULL)));
}

