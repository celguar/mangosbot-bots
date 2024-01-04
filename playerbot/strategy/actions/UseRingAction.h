#pragma once
#include "UseItemAction.h"

namespace ai
{
    class UseRingAction : public UseItemAction 
    {
    public:
        UseRingAction(PlayerbotAI* ai) : UseItemAction(ai, "use ring", true) {}
        virtual bool Execute(Event& event) override;
        virtual bool isPossible();
        virtual bool isUseful() { return UseItemAction::isUseful() && !bot->HasStealthAura(); }
    };
}