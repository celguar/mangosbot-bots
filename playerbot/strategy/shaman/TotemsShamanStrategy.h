#pragma once

#include "GenericShamanStrategy.h"

namespace ai
{
    class TotemsShamanStrategy : public GenericShamanStrategy
    {
    public:
        TotemsShamanStrategy(PlayerbotAI* ai);

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "totems"; }
    };
}
