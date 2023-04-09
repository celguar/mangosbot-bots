#pragma once
#include "../triggers/GenericTriggers.h"

namespace ai
{
    DEFLECT_TRIGGER(FireWardTrigger, "fire ward");
    DEFLECT_TRIGGER(FrostWardTrigger, "frost ward");

    class BlinkTrigger : public Trigger
    {
    public:
        BlinkTrigger(PlayerbotAI* ai) : Trigger(ai, "blink", 2) {}
        virtual bool IsActive()
        {
            return bot->HasAuraType(SPELL_AURA_MOD_ROOT) ||
                bot->HasAuraType(SPELL_AURA_MOD_STUN);
        }
    };

    class ArcaneIntellectOnPartyTrigger : public BuffOnPartyTrigger {
    public:
        ArcaneIntellectOnPartyTrigger(PlayerbotAI* ai) : BuffOnPartyTrigger(ai, "arcane intellect", 2) {}

        virtual bool IsActive() { return BuffOnPartyTrigger::IsActive() && !ai->HasAura("arcane brilliance", GetTarget()); }
    };

    class ArcaneIntellectTrigger : public BuffTrigger {
    public:
        ArcaneIntellectTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "arcane intellect", 2) {}

        virtual bool IsActive() { return BuffTrigger::IsActive() && !ai->HasAura("arcane brilliance", GetTarget()); }
    };

    class MageArmorTrigger : public BuffTrigger {
    public:
        MageArmorTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "mage armor", 5) {}
        virtual bool IsActive();
    };

    class LivingBombTrigger : public DebuffTrigger {
    public:
        LivingBombTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "living bomb") {}
	};

    class FireballTrigger : public DebuffTrigger {
    public:
        FireballTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "fireball") {}
	};

    class PyroblastTrigger : public DebuffTrigger {
    public:
        PyroblastTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "pyroblast", 10) {}
    };

    class MissileBarrageTrigger : public HasAuraTrigger {
    public:
        MissileBarrageTrigger(PlayerbotAI* ai) : HasAuraTrigger(ai, "missile barrage") {}
    };

    class ArcaneBlastTrigger : public BuffTrigger {
    public:
        ArcaneBlastTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "arcane blast") {}
    };

    class CounterspellInterruptSpellTrigger : public InterruptSpellTrigger
    {
    public:
        CounterspellInterruptSpellTrigger(PlayerbotAI* ai) : InterruptSpellTrigger(ai, "counterspell") {}
    };

    class CombustionTrigger : public BoostTrigger
    {
    public:
        CombustionTrigger(PlayerbotAI* ai) : BoostTrigger(ai, "combustion") {}
    };

    class IcyVeinsTrigger : public BoostTrigger
    {
    public:
        IcyVeinsTrigger(PlayerbotAI* ai) : BoostTrigger(ai, "icy veins") {}
    };

    BOOST_TRIGGER(WaterElementalBoostTrigger, "summon water elemental");

    class PolymorphTrigger : public HasCcTargetTrigger
    {
    public:
        PolymorphTrigger(PlayerbotAI* ai) : HasCcTargetTrigger(ai, "polymorph") {}
    };

    class RemoveCurseTrigger : public NeedCureTrigger
    {
    public:
        RemoveCurseTrigger(PlayerbotAI* ai) : NeedCureTrigger(ai, "remove curse", DISPEL_CURSE) {}
    };

    class PartyMemberRemoveCurseTrigger : public PartyMemberNeedCureTrigger
    {
    public:
        PartyMemberRemoveCurseTrigger(PlayerbotAI* ai) : PartyMemberNeedCureTrigger(ai, "remove curse", DISPEL_CURSE) {}
    };

    class SpellstealTrigger : public TargetAuraDispelTrigger
    {
    public:
        SpellstealTrigger(PlayerbotAI* ai) : TargetAuraDispelTrigger(ai, "spellsteal", DISPEL_MAGIC) {}
    };

    class CounterspellEnemyHealerTrigger : public InterruptEnemyHealerTrigger
    {
    public:
        CounterspellEnemyHealerTrigger(PlayerbotAI* ai) : InterruptEnemyHealerTrigger(ai, "counterspell") {}
    };

    class ArcanePowerTrigger : public BuffTrigger
    {
    public:
        ArcanePowerTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "arcane power") {}
    };

    class PresenceOfMindTrigger : public BuffTrigger
    {
    public:
        PresenceOfMindTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "presence of mind") {}
    };

    class ManaShieldTrigger : public BuffTrigger
    {
    public:
        ManaShieldTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "mana shield", 5) {}
        virtual bool IsActive();
    };

    DEBUFF_TRIGGER_A(IceLanceTrigger, "ice lance");
    DEBUFF_TRIGGER_A(NoImprovedScorchDebuffTrigger, "no improved scorch");

#ifdef MANGOSBOT_TWO
    BUFF_TRIGGER_A(HotStreakTrigger, "hot streak");
#endif

#ifndef MANGOSBOT_ZERO
    class ColdSnapTrigger : public Trigger
    {
    public:
        ColdSnapTrigger(PlayerbotAI* ai) : Trigger(ai, "cold snap", 2) {}
        virtual bool IsActive()
        {
            return !bot->IsSpellReady(12472)    //icy veins on cooldown
                && bot->IsSpellReady(11958);    //cold snap not on cooldown
        }
    };
#endif
}
