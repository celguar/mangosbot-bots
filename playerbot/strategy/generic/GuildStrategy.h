#include "../generic/NonCombatStrategy.h"
#pragma once

namespace ai
{
    class GuildStrategy : public NonCombatStrategy
    {
    public:
        GuildStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}
        virtual string getName() { return "guild"; }
        virtual int GetType() { return STRATEGY_TYPE_GENERIC; }

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
    };



}
