#pragma once

namespace ai
{
    class SayStrategy : public Strategy
    {
    public:
        SayStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "say"; }
    };


}
