#include "../generic/NonCombatStrategy.h"
#pragma once

namespace ai
{
    class AttackEnemyPlayersStrategy : public Strategy
    {
    public:
        AttackEnemyPlayersStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        virtual string getName() { return "pvp"; }

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
    };

}
