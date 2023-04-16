#pragma once

#include "GenericMageStrategy.h"
#include "../generic/CombatStrategy.h"

namespace ai
{
    class FireMageStrategy : public GenericMageStrategy
    {
    public:
        FireMageStrategy(PlayerbotAI* ai);
        string getName() override { return "fire"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
        NextAction** GetDefaultCombatActions() override;
    };

    class FireMageAoeStrategy : public CombatStrategy
    {
    public:
        FireMageAoeStrategy(PlayerbotAI* ai) : CombatStrategy(ai) {}
        string getName() override { return "fire aoe"; }

    private:
        virtual void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };
}
