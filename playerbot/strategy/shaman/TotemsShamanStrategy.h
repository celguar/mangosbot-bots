#pragma once
#include "GenericShamanStrategy.h"

namespace ai
{
    class TotemsShamanStrategy : public GenericShamanStrategy
    {
    public:
        TotemsShamanStrategy(PlayerbotAI* ai) : GenericShamanStrategy(ai) {}
        string getName() override { return "totems"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };

    class ShamanTotemBarElementsStrategy : public GenericShamanStrategy
    {
    public:
        ShamanTotemBarElementsStrategy(PlayerbotAI* ai) : GenericShamanStrategy(ai) {}
        string getName() { return "totembar elements"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };

    class ShamanTotemBarAncestorsStrategy : public GenericShamanStrategy
    {
    public:
        ShamanTotemBarAncestorsStrategy(PlayerbotAI* ai) : GenericShamanStrategy(ai) {}
        string getName() { return "totembar ancestors"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;        
    };

    class ShamanTotemBarSpiritsStrategy : public GenericShamanStrategy
    {
    public:
        ShamanTotemBarSpiritsStrategy(PlayerbotAI* ai) : GenericShamanStrategy(ai) {}
        string getName() { return "totembar spirits"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;        
    };
}
