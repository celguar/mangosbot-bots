#pragma once

namespace ai
{
    class RpgActionMultiplier : public Multiplier
    {
    public:
        RpgActionMultiplier(PlayerbotAI* ai) : Multiplier(ai, "rpg action") {}

    public:
        virtual float GetValue(Action* action);
    };

    class RpgStrategy : public Strategy
    {
    public:
        RpgStrategy(PlayerbotAI* ai);
        virtual string getName() { return "rpg"; }

    public:
        virtual NextAction** getDefaultActions();
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        void InitMultipliers(std::list<Multiplier*>& multipliers);
    };

}
