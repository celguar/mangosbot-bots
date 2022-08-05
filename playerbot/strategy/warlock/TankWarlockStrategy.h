#pragma once

#include "GenericWarlockStrategy.h"

namespace ai
{
    class TankWarlockStrategy : public GenericWarlockStrategy
    {
    public:
        TankWarlockStrategy(PlayerbotAI* ai);
        virtual string getName() { return "tank"; }

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual NextAction** getDefaultActions();
    };
}
