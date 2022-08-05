#pragma once
#include "PassTroughStrategy.h"

namespace ai
{
    class DeadStrategy : public PassTroughStrategy
    {
    public:
        DeadStrategy(PlayerbotAI* ai);

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "dead"; }
    };
}
