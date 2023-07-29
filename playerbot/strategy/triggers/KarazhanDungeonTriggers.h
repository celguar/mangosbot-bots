#pragma once
#include "DungeonTriggers.h"
#include "GenericTriggers.h"

namespace ai
{
    class KarazhanEnterDungeonTrigger : public EnterDungeonTrigger
    {
    public:
        KarazhanEnterDungeonTrigger(PlayerbotAI* ai) : EnterDungeonTrigger(ai, "enter karazhan", "karazhan", 532) {}
    };

    class KarazhanLeaveDungeonTrigger : public LeaveDungeonTrigger
    {
    public:
        KarazhanLeaveDungeonTrigger(PlayerbotAI* ai) : LeaveDungeonTrigger(ai, "leave karazhan", "karazhan", 532) {}
    };

    class NetherspiteStartFightTrigger : public StartBossFightTrigger
    {
    public:
        NetherspiteStartFightTrigger(PlayerbotAI* ai) : StartBossFightTrigger(ai, "start netherspite fight", "netherspite", 15689) {}
    };

    class NetherspiteEndFightTrigger : public EndBossFightTrigger
    {
    public:
        NetherspiteEndFightTrigger(PlayerbotAI* ai) : EndBossFightTrigger(ai, "end netherspite fight", "netherspite", 15689) {}
    };

    class VoidZoneTooCloseTrigger : public CloseToCreatureTrigger
    {
    public:
        VoidZoneTooCloseTrigger(PlayerbotAI* ai) : CloseToCreatureTrigger(ai, "void zone too close", 16697, 5.0f) {}
    };
}