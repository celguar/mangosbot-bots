#pragma once
#include "../triggers/GenericTriggers.h"

namespace ai
{
    BUFF_TRIGGER(BattleShoutTrigger, "battle shout");
    BUFF_TRIGGER(BattleStanceTrigger, "battle stance");
    BUFF_TRIGGER(DefensiveStanceTrigger, "defensive stance");
    BUFF_TRIGGER(BerserkerStanceTrigger, "berserker stance");
    BUFF_TRIGGER(ShieldBlockTrigger, "shield block");
    BUFF_TRIGGER_A(CommandingShoutTrigger, "commanding shout");

    DEBUFF_TRIGGER(RendDebuffTrigger, "rend");
    DEBUFF_TRIGGER(DisarmDebuffTrigger, "disarm");
    DEBUFF_TRIGGER_A(SunderArmorDebuffTrigger, "sunder armor");
    DEBUFF_TRIGGER(MortalStrikeDebuffTrigger, "mortal strike");
    DEBUFF_ENEMY_TRIGGER(RendDebuffOnAttackerTrigger, "rend");
    CAN_CAST_TRIGGER(DevastateAvailableTrigger, "devastate");
    CAN_CAST_TRIGGER(RevengeAvailableTrigger, "revenge");
    CAN_CAST_TRIGGER(OverpowerAvailableTrigger, "overpower");
    BUFF_TRIGGER(RampageAvailableTrigger, "rampage");
    BUFF_TRIGGER_A(BloodrageBuffTrigger, "bloodrage");
    CAN_CAST_TRIGGER(VictoryRushTrigger, "victory rush");
    HAS_AURA_TRIGGER(SwordAndBoardTrigger, "sword and board");
    SNARE_TRIGGER(ConcussionBlowTrigger, "concussion blow");
    SNARE_TRIGGER(HamstringTrigger, "hamstring");
    SNARE_TRIGGER(MockingBlowTrigger, "mocking blow");
    SNARE_TRIGGER(ThunderClapSnareTrigger, "thunder clap");
    DEBUFF_TRIGGER(ThunderClapTrigger, "thunder clap");
    SNARE_TRIGGER(TauntSnareTrigger, "taunt");
    SNARE_TRIGGER(InterceptSnareTrigger, "intercept");
    CD_TRIGGER(InterceptCanCastTrigger, "intercept");
    SNARE_TRIGGER(ShockwaveSnareTrigger, "shockwave");
    DEBUFF_TRIGGER(ShockwaveTrigger, "shockwave");
    BOOST_TRIGGER(DeathWishTrigger, "death wish");
    BOOST_TRIGGER(RecklessnessTrigger, "recklessness");
    BUFF_TRIGGER(BloodthirstBuffTrigger, "bloodthirst");
    INTERRUPT_HEALER_TRIGGER(ShieldBashInterruptEnemyHealerSpellTrigger, "shield bash");
    INTERRUPT_TRIGGER(ShieldBashInterruptSpellTrigger, "shield bash");
    INTERRUPT_HEALER_TRIGGER(PummelInterruptEnemyHealerSpellTrigger, "pummel");
    INTERRUPT_TRIGGER(PummelInterruptSpellTrigger, "pummel");
    INTERRUPT_HEALER_TRIGGER(InterceptInterruptEnemyHealerSpellTrigger, "intercept");
    INTERRUPT_TRIGGER(InterceptInterruptSpellTrigger, "intercept");
    DEFLECT_TRIGGER(SpellReflectionTrigger, "spell reflection");
    HAS_AURA_TRIGGER(SuddenDeathTrigger, "sudden death");
    HAS_AURA_TRIGGER(SlamInstantTrigger, "slam!");
    HAS_AURA_TRIGGER(TasteForBloodTrigger, "taste for blood");

    class BerserkerRageBuffTrigger : public TargetOfFearCastTrigger
    {
    public:
        BerserkerRageBuffTrigger(PlayerbotAI* ai) : TargetOfFearCastTrigger(ai) {}
        bool IsActive() override
        {
            // Check for spell cooldown
            uint32 spellid = AI_VALUE2(uint32, "spell id", "berserker rage");
            if (spellid && bot->IsSpellReady(spellid))
            {
                return TargetOfFearCastTrigger::IsActive();
            }

            return false;
        }
    };
}
