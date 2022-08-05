#pragma once

#include "../Strategy.h"
#include "../generic/NonCombatStrategy.h"

namespace ai
{
    class ShamanNonCombatStrategy : public NonCombatStrategy
    {
    public:
        ShamanNonCombatStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual void InitMultipliers(std::vector<Multiplier*> &multipliers);
        virtual string getName() { return "nc"; }

    };
}
