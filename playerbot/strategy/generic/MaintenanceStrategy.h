#include "../generic/NonCombatStrategy.h"
#pragma once

namespace ai
{
    class MaintenanceStrategy : public NonCombatStrategy
    {
    public:
        MaintenanceStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}
        virtual string getName() { return "maintenance"; }
        virtual int GetType() { return STRATEGY_TYPE_NONCOMBAT; }
        NextAction** getDefaultActions();

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
    };



}
