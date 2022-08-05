#pragma once

#include "../generic/NonCombatStrategy.h"

namespace ai
{
    class GenericHunterNonCombatStrategy : public NonCombatStrategy
    {
    public:
        GenericHunterNonCombatStrategy(PlayerbotAI* ai);
        virtual string getName() { return "nc"; }

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
    };

    class HunterPetStrategy : public NonCombatStrategy
    {
    public:
        HunterPetStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}
        virtual string getName() { return "pet"; }

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
    };

}
