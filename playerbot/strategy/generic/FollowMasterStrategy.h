#include "../generic/NonCombatStrategy.h"
#pragma once

namespace ai
{
    class FollowMasterStrategy : public NonCombatStrategy
    {
    public:
        FollowMasterStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}
        virtual string getName() { return "follow"; }
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);

    };

}
