#pragma once

#include "GenericPriestStrategy.h"

namespace ai
{
    class HealPriestStrategy : public GenericPriestStrategy
    {
    public:
        HealPriestStrategy(PlayerbotAI* ai);

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual NextAction** getDefaultActions();
        virtual string getName() { return "heal"; }
		virtual int GetType() { return STRATEGY_TYPE_HEAL; }
    };
}
