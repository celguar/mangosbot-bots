#pragma once

#include "../generic/NonCombatStrategy.h"

namespace ai
{
    class GenericDruidNonCombatStrategy : public NonCombatStrategy
    {
    public:
        GenericDruidNonCombatStrategy(PlayerbotAI* ai);
        virtual string getName() { return "nc"; }

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
    };

    class GenericDruidBuffStrategy : public NonCombatStrategy
    {
    public:
        GenericDruidBuffStrategy(PlayerbotAI* ai);
        virtual string getName() { return "buff"; }

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
    };
}
