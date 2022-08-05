#pragma once
#include "PassTroughStrategy.h"

namespace ai
{
    class DuelStrategy : public PassTroughStrategy
    {
    public:
        DuelStrategy(PlayerbotAI* ai);

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "duel"; }
    };

    class StartDuelStrategy : public Strategy
    {
    public:
        StartDuelStrategy(PlayerbotAI* ai);

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> & triggers);
        virtual string getName() { return "start duel"; }
    };
}
