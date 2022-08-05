#pragma once

#include "../generic/CombatStrategy.h"
#include "GenericDruidStrategy.h"

namespace ai
{
    class MeleeDruidStrategy : public CombatStrategy
    {
    public:
        MeleeDruidStrategy(PlayerbotAI* ai);

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "melee"; }
        virtual NextAction** getDefaultActions();
    };

}
