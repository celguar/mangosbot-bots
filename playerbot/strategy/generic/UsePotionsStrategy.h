#pragma once

namespace ai
{
    class UsePotionsStrategy : public Strategy
    {
    public:
        UsePotionsStrategy(PlayerbotAI* ai);

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "potions"; }
    };

}
