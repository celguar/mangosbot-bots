#pragma once

namespace ai
{
    class FleeStrategy : public Strategy
    {
    public:
        FleeStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "flee"; };
    };

    class FleeFromAddsStrategy : public Strategy
    {
    public:
        FleeFromAddsStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "flee from adds"; };
    };

}
