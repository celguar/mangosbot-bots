#pragma once

#include "GenericPaladinStrategy.h"

namespace ai
{
    class TankPaladinStrategy : public GenericPaladinStrategy
    {
    public:
        TankPaladinStrategy(PlayerbotAI* ai);

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "tank"; }
        virtual NextAction** getDefaultActions();
		virtual int GetType() { return STRATEGY_TYPE_TANK | STRATEGY_TYPE_MELEE; }
    };
}
