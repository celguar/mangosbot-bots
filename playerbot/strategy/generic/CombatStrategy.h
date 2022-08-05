#pragma once

namespace ai
{
    class CombatStrategy : public Strategy
    {
    public:
        CombatStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual int GetType() { return STRATEGY_TYPE_COMBAT; }
    };

    class AvoidAoeStrategy : public Strategy
    {
    public:
        AvoidAoeStrategy(PlayerbotAI* ai);
        virtual string getName() { return "avoid aoe"; }
        virtual void InitMultipliers(std::list<Multiplier*>& multipliers);
        virtual void InitTriggers(std::vector<TriggerNode*> & triggers);
    };
}
