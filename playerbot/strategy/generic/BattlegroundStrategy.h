#pragma once
#include "PassTroughStrategy.h"

namespace ai
{
    class BGStrategy : public PassTroughStrategy
    {
    public:
        BGStrategy(PlayerbotAI* ai);
        virtual int GetType() { return STRATEGY_TYPE_NONCOMBAT; }
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "bg"; }
    };

    class BattlegroundStrategy : public Strategy
    {
    public:
        BattlegroundStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        virtual int GetType() { return STRATEGY_TYPE_NONCOMBAT; }
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "battleground"; }
    };

    class WarsongStrategy : public Strategy
    {
    public:
        WarsongStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        virtual int GetType() { return STRATEGY_TYPE_GENERIC; }
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "warsong"; }
    };

    class AlteracStrategy : public Strategy
    {
    public:
        AlteracStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        virtual int GetType() { return STRATEGY_TYPE_GENERIC; }
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "alterac"; }
    };

    class ArathiStrategy : public Strategy
    {
    public:
        ArathiStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        virtual int GetType() { return STRATEGY_TYPE_GENERIC; }
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "arathi"; }
    };

    class EyeStrategy : public Strategy
    {
    public:
        EyeStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        virtual int GetType() { return STRATEGY_TYPE_GENERIC; }
        virtual void InitTriggers(std::vector<TriggerNode*> & triggers);
        virtual string getName() { return "eye"; }
    };

    class IsleStrategy : public Strategy
    {
    public:
        IsleStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        virtual int GetType() { return STRATEGY_TYPE_GENERIC; }
        virtual void InitTriggers(std::vector<TriggerNode*> & triggers);
        virtual string getName() { return "isle"; }
    };

    class ArenaStrategy : public Strategy
    {
    public:
        ArenaStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        virtual int GetType() { return STRATEGY_TYPE_GENERIC; }
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "arena"; }
    };
}
