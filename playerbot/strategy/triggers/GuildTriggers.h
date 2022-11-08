#pragma once
#include "../Trigger.h"

namespace ai
{	
    class PetitionTurnInTrigger : public Trigger {
    public:
        PetitionTurnInTrigger(PlayerbotAI* ai) :
            Trigger(ai) {}

        bool IsActive() { return !bot->GetGuildId() && AI_VALUE2(uint32, "item count", chat->formatQItem(5863)) && AI_VALUE(uint8, "petition signs") >= sWorld.getConfig(CONFIG_UINT32_MIN_PETITION_SIGNS); };
    };

    class BuyTabardTrigger : public Trigger {
    public:
        BuyTabardTrigger(PlayerbotAI* ai) :
            Trigger(ai) {}

        bool IsActive();
    };

    class LeaveLargeGuildTrigger : public Trigger {
    public:
        LeaveLargeGuildTrigger(PlayerbotAI* ai) :
            Trigger(ai) {}

        bool IsActive();
    };
}