#include "../generic/NonCombatStrategy.h"
#pragma once

namespace ai
{
    class TankAssistStrategy : public Strategy
    {
    public:
        TankAssistStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        virtual string getName() { return "tank assist"; }
        virtual int GetType() { return STRATEGY_TYPE_TANK; }

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
    };

}
