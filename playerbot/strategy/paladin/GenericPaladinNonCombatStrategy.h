#pragma once

#include "../generic/NonCombatStrategy.h"

namespace ai
{
    class GenericPaladinNonCombatStrategy : public NonCombatStrategy
    {
    public:
        GenericPaladinNonCombatStrategy(PlayerbotAI* ai);
        virtual string getName() { return "nc"; }
    
    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
   };
}
