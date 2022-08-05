#pragma once

#include "GenericWarriorStrategy.h"

namespace ai
{
    class ArmsWarriorStrategy : public GenericWarriorStrategy
    {
    public:
        ArmsWarriorStrategy(PlayerbotAI* ai);

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "arms"; }
        virtual NextAction** getDefaultActions();
        virtual int GetType() { return STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_DPS | STRATEGY_TYPE_MELEE; }
    };
}
