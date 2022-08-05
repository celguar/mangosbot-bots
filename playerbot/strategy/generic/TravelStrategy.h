#pragma once

namespace ai
{
    class TravelStrategy : public Strategy
    {
    public:
        TravelStrategy(PlayerbotAI* ai);
        virtual string getName() { return "travel"; }

    public:
        virtual NextAction** getDefaultActions();
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
    };

    class ExploreStrategy : public Strategy
    {
    public:
        ExploreStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        virtual string getName() { return "explore"; }
    };

    class MapStrategy : public Strategy
    {
    public:
        MapStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        virtual string getName() { return "map"; }
    };

    class MapFullStrategy : public Strategy
    {
    public:
        MapFullStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        virtual string getName() { return "map full"; }
    };
}
