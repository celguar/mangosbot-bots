#pragma once

#include "../Strategy.h"
#include "../generic/CombatStrategy.h"

namespace ai
{
    class GenericRogueStrategy : public CombatStrategy
    {
    public:
        GenericRogueStrategy(PlayerbotAI* ai);

    public:
        virtual void InitTriggers(std::vector<TriggerNode*>& triggers);
        virtual string getName() { return "generic"; }
    };

    class DpsRogueStrategy : public GenericRogueStrategy
    {
    public:
        DpsRogueStrategy(PlayerbotAI* ai);

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "dps"; }
    };

    class AssassinationRogueStrategy : public GenericRogueStrategy
    {
    public:
        AssassinationRogueStrategy(PlayerbotAI* ai);

    public:
        virtual void InitTriggers(std::vector<TriggerNode*>& triggers);
        virtual string getName() { return "assassin"; }
    };

    class CombatRogueStrategy : public GenericRogueStrategy
    {
    public:
        CombatRogueStrategy(PlayerbotAI* ai);

    public:
        virtual void InitTriggers(std::vector<TriggerNode*>& triggers);
        virtual string getName() { return "combat"; }
    };


    class StealthedRogueStrategy : public Strategy
    {
    public:
        StealthedRogueStrategy(PlayerbotAI* ai);

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "stealthed"; }
    };

    class StealthStrategy : public Strategy
    {
    public:
        StealthStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        //virtual int GetType() { return STRATEGY_TYPE_NONCOMBAT; }
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "stealth"; }
    };

    class RogueAoeStrategy : public Strategy
    {
    public:
        RogueAoeStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "aoe"; }
    };

    class RogueBoostStrategy : public Strategy
    {
    public:
        RogueBoostStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "boost"; }
    };

    class RogueCcStrategy : public Strategy
    {
    public:
        RogueCcStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "cc"; }
    };
}
