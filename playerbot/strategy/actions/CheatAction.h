#pragma once
#include "GenericActions.h"

namespace ai
{
    class CheatAction : public ChatCommandAction
    {
    public:
        CheatAction(PlayerbotAI* ai) : ChatCommandAction(ai, "cheat") {}
        virtual bool Execute(Event& event) override;
    private:
        static BotCheatMask GetCheatMask(string cheat);
        static string GetCheatName(BotCheatMask cheatMask);
        void ListCheats();
    };
}
