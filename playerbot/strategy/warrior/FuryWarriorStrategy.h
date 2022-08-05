#pragma once

#include "GenericWarriorStrategy.h"

namespace ai
{
    class FuryWarriorStrategy : public GenericWarriorStrategy
    {
    public:
        FuryWarriorStrategy(PlayerbotAI* ai);

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "fury"; }
        virtual NextAction** getDefaultActions();
        virtual int GetType() { return STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_DPS | STRATEGY_TYPE_MELEE; }
    };
}
