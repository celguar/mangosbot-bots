#pragma once
#include "PassTroughStrategy.h"

namespace ai
{
    class NonCombatStrategy : public Strategy
    {
    public:
        NonCombatStrategy(PlayerbotAI* ai) : Strategy(ai) {}
		virtual int GetType() { return STRATEGY_TYPE_NONCOMBAT; }
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
    };

    class CollisionStrategy : public Strategy
    {
    public:
        CollisionStrategy(PlayerbotAI* ai) : Strategy(ai) {}
		virtual int GetType() { return STRATEGY_TYPE_NONCOMBAT; }
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "collision"; }
    };

    class MountStrategy : public Strategy
    {
    public:
        MountStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        virtual int GetType() { return STRATEGY_TYPE_NONCOMBAT; }
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "mount"; }
    };

    class AttackTaggedStrategy : public Strategy
    {
    public:
        AttackTaggedStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        virtual int GetType() { return STRATEGY_TYPE_NONCOMBAT; }
        virtual string getName() { return "attack tagged"; }
    };
}
