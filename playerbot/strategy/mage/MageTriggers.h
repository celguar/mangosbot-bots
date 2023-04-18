#pragma once
#include "../triggers/GenericTriggers.h"
#include "../triggers/GlyphTriggers.h"

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

    class IceBarrierTrigger : public BuffTrigger
    {
    public:
        IceBarrierTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "ice barrier", 5) {}
    };

    class FireSpellsLocked : public Trigger
    {
    public:
        FireSpellsLocked(PlayerbotAI* ai) : Trigger(ai, "fire spells locked", 2) {}
        virtual bool IsActive()
        {
            return !bot->IsSpellReady(133)      //fireball
                && !bot->IsSpellReady(2948);    //scorch
        }
    };

    DEBUFF_TRIGGER_A(IceLanceTrigger, "ice lance");
    

#ifdef MANGOSBOT_TWO
    BUFF_TRIGGER(MirrorImageTrigger, "mirror image");
    BUFF_TRIGGER_A(HotStreakTrigger, "hot streak");
    BUFF_TRIGGER_A(FireballOrFrostfireBoltFreeTrigger, "fireball!");
    DEBUFF_TRIGGER_A(NoImprovedScorchDebuffTrigger, "no improved scorch");
    HAS_AURA_TRIGGER(FingersOfFrostTrigger, "fingers of frost");

    class LearnGlyphOfFireballTrigger : public LearnGlyphTrigger
    {
    public:
        LearnGlyphOfFireballTrigger(PlayerbotAI* ai) : LearnGlyphTrigger(ai,
            "learn glyph of fireball",
            56975,                          //Glyph of Fireball
            -1,                             //No required spell
            15,                             //Required 15 lvl
            "fire"                          //Required fire strategy
        ) {}
    };

    class RemoveGlyphOfFireballTrigger : public RemoveGlyphTrigger
    {
    public:
        RemoveGlyphOfFireballTrigger(PlayerbotAI* ai) : RemoveGlyphTrigger(ai,
            "remove glyph of fireball",
            56975,                          //Glyph of Fireball
            -1,                             //No required spell
            15,                             //Required 15 lvl
            "fire"                          //Required fire strategy
        ) {}
    };

    class RemoveGlyphOfFrostboltTrigger : public RemoveGlyphTrigger
    {
    public:
        RemoveGlyphOfFrostboltTrigger(PlayerbotAI* ai) : RemoveGlyphTrigger(ai,
            "remove glyph of frostbolt",
            56370,                          //Glyph of Frostbolt
            -1,                             //No required spell
            15,                             //Required 15 lvl
            "frost"                         //Required frost strategy
        ) {}
    };

    class LearnGlyphOfFrostboltTrigger : public LearnGlyphTrigger
    {
    public:
        LearnGlyphOfFrostboltTrigger(PlayerbotAI* ai) : LearnGlyphTrigger(ai,
            "learn glyph of frostbolt",
            56370,                          //Glyph of Frostbolt
            -1,                             //No required spell
            15,                             //Required 15 lvl
            "frost"                         //Required frost strategy
        ) {}
    };

    class LearnGlyphOfLivingBombTrigger : public LearnGlyphTrigger
    {
    public:
        LearnGlyphOfLivingBombTrigger(PlayerbotAI* ai) : LearnGlyphTrigger(ai,
            "learn glyph of living bomb",
            64275,                          //Glyph of Living Bomb
            44457,                          //Required Living Bomb talent
            45,                             //Required 45 lvl
            "fire"                          //Required fire strategy
        ) {}
    };

    class RemoveGlyphOfLivingBombTrigger : public RemoveGlyphTrigger
    {
    public:
        RemoveGlyphOfLivingBombTrigger(PlayerbotAI* ai) : RemoveGlyphTrigger(ai,
            "remove glyph of living bomb",
            64275,                          //Glyph of Living Bomb
            44457,                          //Required Living Bomb talent
            45,                             //Required 45 lvl
            "fire"                          //Required fire strategy
        ) {}
    };

    class LearnGlyphOfMirrorImageTrigger : public LearnGlyphTrigger
    {
    public:
        LearnGlyphOfMirrorImageTrigger(PlayerbotAI* ai) : LearnGlyphTrigger(ai,
            "learn glyph of mirror image",
            63093,                          //Glyph of Mirror Image
            55342,                          //Required Mirror Image
            80,                             //Required 80 lvl
            "frost"                         //Required frost strategy
        ) {}
    };

    class RemoveGlyphOfMirrorImageTrigger : public RemoveGlyphTrigger
    {
    public:
        RemoveGlyphOfMirrorImageTrigger(PlayerbotAI* ai) : RemoveGlyphTrigger(ai,
            "remove glyph of mirror image",
            63093,                          //Glyph of Mirror Image
            55342,                          //Required Mirror Image
            80,                             //Required 80 lvl
            "frost"                         //Required frost strategy
        ) {}
    };

    class LearnGlyphOfMoltenArmorTrigger : public LearnGlyphTrigger
    {
    public:
        LearnGlyphOfMoltenArmorTrigger(PlayerbotAI* ai) : LearnGlyphTrigger(ai,
            "learn glyph of molten armor", 
            42751,                          //Glyph of Molten Armor
            30482,                          //Required Molten Armor Rank 1
            62,                             //Required 62 lvl
            ""                              //No strategy required
            ) {}
    };

    class RemoveGlyphOfMoltenArmorTrigger : public RemoveGlyphTrigger
    {
    public:
        RemoveGlyphOfMoltenArmorTrigger(PlayerbotAI* ai) : RemoveGlyphTrigger(ai,
            "remove glyph of molten armor",
            42751,                          //Glyph of Molten Armor
            30482,                          //Required Molten Armor Rank 1
            62,                             //Required 62 lvl
            ""                              //No strategy required
        ) {}
    };
#else
    class NoFireVulnerabilityTrigger : public Trigger
    {
    public:
        NoFireVulnerabilityTrigger(PlayerbotAI* ai) : Trigger(ai, "no fire vulnerability") {}
        virtual bool IsActive()
        {
            //Required Improved Scorch talent
            if (!bot->HasSpell(11095) && !bot->HasSpell(12872) && !bot->HasSpell(12873))
                return false;

            //Required non-player target which dont have max stacks of fire vulnerability expiring in less than 7 seconds
            Unit* target = GetTarget();
            if (target && (target->IsPlayer() || ai->HasAura("fire vulnerability", target, true, false, -1, false, 7000)))
                return false;

            return true;
        }
        virtual string GetTargetName() { return "current target"; }
    };
#endif

#ifndef MANGOSBOT_ZERO
    class LivingBombTrigger : public DebuffTrigger {
    public:
        LivingBombTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "living bomb") {}
    };

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
#else
    class ColdSnapTrigger : public Trigger
    {
    public:
        ColdSnapTrigger(PlayerbotAI* ai) : Trigger(ai, "cold snap", 2) {}
        virtual bool IsActive()
        {
            return !bot->IsSpellReady(11958)    //Ice Block on cooldown
                && bot->IsSpellReady(12472);    //Cold Snap not on cooldown
        }
    };
#endif
}
