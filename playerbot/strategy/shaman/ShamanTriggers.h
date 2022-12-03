#pragma once
#include "../triggers/GenericTriggers.h"

namespace ai
{
    class ShamanWeaponTrigger : public BuffTrigger {
    public:
        ShamanWeaponTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "rockbiter weapon") {}
        virtual bool IsActive();
    private:
        static list<string> spells;
    };

    class ReadyToRemoveTotemsTrigger : public Trigger {
    public:
        ReadyToRemoveTotemsTrigger(PlayerbotAI* ai) : Trigger(ai, "ready to remove totems", 5) {}

        virtual bool IsActive()
        {
            // Avoid removing any of the big cooldown totems.
            return AI_VALUE(bool, "have any totem")
                && !AI_VALUE2(bool, "has totem", "mana tide totem")
                && !AI_VALUE2(bool, "has totem", "earth elemental totem")
                && !AI_VALUE2(bool, "has totem", "fire elemental totem");
        }
    };

    class TotemsAreNotSummonedTrigger : public Trigger {
    public:
        TotemsAreNotSummonedTrigger(PlayerbotAI* ai) : Trigger(ai, "no totems summoned", 5) {}

        virtual bool IsActive()
        {
            return !AI_VALUE(bool, "have any totem");
        }
    };

    class TotemTrigger : public Trigger {
    public:
        TotemTrigger(PlayerbotAI* ai, string spell, int attackerCount = 0) : Trigger(ai, spell), attackerCount(attackerCount) {}

        virtual bool IsActive()
		{
            return AI_VALUE(uint8, "attackers count") >= attackerCount && !AI_VALUE2(bool, "has totem", name);
        }

    protected:
        int attackerCount;
    };

    class FireTotemTrigger : public Trigger {
    public:
        FireTotemTrigger(PlayerbotAI* ai) : Trigger(ai, "trigger spec appropriate fire totem") {}
        virtual bool IsActive()
        {
            return !AI_VALUE2(bool, "has totem", "searing totem") &&
                !AI_VALUE2(bool, "has totem", "magma totem") &&
                !AI_VALUE2(bool, "has totem", "frost resistance totem") &&
                !AI_VALUE2(bool, "has totem", "flametongue totem") &&
                !AI_VALUE2(bool, "has totem", "totem of wrath") &&
                !AI_VALUE2(bool, "has totem", "fire elemental totem");
        }
    };

    class FireTotemAoeTrigger : public Trigger {
    public:
        FireTotemAoeTrigger(PlayerbotAI* ai) : Trigger(ai, "trigger spec appropriate fire totem aoe", 2) {}
        virtual bool IsActive()
        {
            return AI_VALUE(uint8, "attackers count") >= 3 &&
                !AI_VALUE2(bool, "has totem", "searing totem") &&
                !AI_VALUE2(bool, "has totem", "magma totem") &&
                !AI_VALUE2(bool, "has totem", "frost resistance totem") &&
                !AI_VALUE2(bool, "has totem", "flametongue totem") &&
                !AI_VALUE2(bool, "has totem", "totem of wrath") &&
                !AI_VALUE2(bool, "has totem", "fire elemental totem");
        }
    };

    class EarthTotemTrigger : public Trigger {
    public:
        EarthTotemTrigger(PlayerbotAI* ai) : Trigger(ai, "trigger spec appropriate earth totem") {}
        virtual bool IsActive()
        {
            return !AI_VALUE2(bool, "has totem", "earthbind totem") &&
                !AI_VALUE2(bool, "has totem", "stoneskin totem") &&
                !AI_VALUE2(bool, "has totem", "stoneclaw totem") &&
                !AI_VALUE2(bool, "has totem", "strength of earth totem") &&
                !AI_VALUE2(bool, "has totem", "tremor totem") &&
                !AI_VALUE2(bool, "has totem", "earth elemental totem");
        }
    };

    class AirTotemTrigger : public Trigger {
    public:
        AirTotemTrigger(PlayerbotAI* ai) : Trigger(ai, "trigger spec appropriate air totem") {}
        virtual bool IsActive()
        {
            return !AI_VALUE2(bool, "has totem", "windfury totem") &&
                !AI_VALUE2(bool, "has totem", "grounding totem") &&
                !AI_VALUE2(bool, "has totem", "sentry totem") &&
                !AI_VALUE2(bool, "has totem", "nature resistance totem") &&
                !AI_VALUE2(bool, "has totem", "wrath of air totem");
        }
    };

    class WaterTotemTrigger : public Trigger {
    public:
        WaterTotemTrigger(PlayerbotAI* ai) : Trigger(ai, "trigger spec appropriate water totem") {}
        virtual bool IsActive()
        {
            return !AI_VALUE2(bool, "has totem", "healing stream totem") &&
                !AI_VALUE2(bool, "has totem", "mana spring totem") &&
                !AI_VALUE2(bool, "has totem", "poison cleansing totem") &&
                !AI_VALUE2(bool, "has totem", "disease cleansing totem") &&
                !AI_VALUE2(bool, "has totem", "mana tide totem") &&
                !AI_VALUE2(bool, "has totem", "fire resistance totem");
        }
    };

    //class WindfuryTotemTrigger : public TotemTrigger {
    //public:
    //    WindfuryTotemTrigger(PlayerbotAI* ai) : TotemTrigger(ai, "windfury totem") {}
    //};

    //class GraceOfAirTotemTrigger : public TotemTrigger
    //{
    //public:
    //    GraceOfAirTotemTrigger(PlayerbotAI* ai) : TotemTrigger(ai, "grace of air totem") {}
    //};

    //class ManaSpringTotemTrigger : public TotemTrigger {
    //public:
    //    ManaSpringTotemTrigger(PlayerbotAI* ai) : TotemTrigger(ai, "mana spring totem") {}
    //    virtual bool IsActive()
    //    {
    //        return AI_VALUE(uint8, "attackers count") >= attackerCount &&
    //                !AI_VALUE2(bool, "has totem", "mana tide totem") &&
    //                !AI_VALUE2(bool, "has totem", name);
    //    }
    //};

    //class FlametongueTotemTrigger : public TotemTrigger {
    //public:
    //    FlametongueTotemTrigger(PlayerbotAI* ai) : TotemTrigger(ai, "flametongue totem") {}
    //};

    //class StrengthOfEarthTotemTrigger : public TotemTrigger {
    //public:
    //    StrengthOfEarthTotemTrigger(PlayerbotAI* ai) : TotemTrigger(ai, "strength of earth totem") {}
    //};

    //class MagmaTotemTrigger : public TotemTrigger {
    //public:
    //    MagmaTotemTrigger(PlayerbotAI* ai) : TotemTrigger(ai, "magma totem", 3) {}
    //};

    //class SearingTotemTrigger : public TotemTrigger {
    //public:
    //    SearingTotemTrigger(PlayerbotAI* ai) : TotemTrigger(ai, "searing totem", 1) {}
    //};

    class WindShearInterruptSpellTrigger : public InterruptSpellTrigger
    {
    public:
        WindShearInterruptSpellTrigger(PlayerbotAI* ai) : InterruptSpellTrigger(ai, "wind shear") {}
    };

    class WaterShieldTrigger : public BuffTrigger
    {
    public:
        WaterShieldTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "water shield") {}
    };

    class LightningShieldTrigger : public BuffTrigger
    {
    public:
        LightningShieldTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "lightning shield") {}
    };

    class PurgeTrigger : public TargetAuraDispelTrigger
    {
    public:
        PurgeTrigger(PlayerbotAI* ai) : TargetAuraDispelTrigger(ai, "purge", DISPEL_MAGIC, 3) {}
        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, "current target");
            if (!target)
                return false;

            if (sServerFacade.IsFriendlyTo(bot, target))
                return false;

            for (uint32 type = SPELL_AURA_NONE; type < TOTAL_AURAS; ++type)
            {
                Unit::AuraList const& auras = target->GetAurasByType((AuraType)type);
                for (Unit::AuraList::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
                {
                    const Aura* aura = *itr;
                    const SpellEntry* entry = aura->GetSpellProto();
                    uint32 spellId = entry->Id;
                    if (!IsPositiveSpell(spellId))
                        continue;

                    vector<uint32> ignoreSpells;
                    ignoreSpells.push_back(17627);
                    ignoreSpells.push_back(24382);
                    ignoreSpells.push_back(22888);
                    ignoreSpells.push_back(24425);
                    ignoreSpells.push_back(16609);
                    ignoreSpells.push_back(22818);
                    ignoreSpells.push_back(22820);
                    ignoreSpells.push_back(15366);
                    ignoreSpells.push_back(15852);
                    ignoreSpells.push_back(22817);
                    ignoreSpells.push_back(17538);
                    ignoreSpells.push_back(11405);
                    ignoreSpells.push_back(7396);
                    ignoreSpells.push_back(17539);

                    if (find(ignoreSpells.begin(), ignoreSpells.end(), spellId) != ignoreSpells.end())
                        continue;

                    /*if (sPlayerbotAIConfig.dispelAuraDuration && aura->GetAuraDuration() && aura->GetAuraDuration() < (int32)sPlayerbotAIConfig.dispelAuraDuration)
                        return false;*/

                    if (ai->canDispel(entry, DISPEL_MAGIC))
                        return true;
                }
            }
            return false;
        }
    };

    class WaterWalkingTrigger : public BuffTrigger {
    public:
        WaterWalkingTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "water walking", 7) {}

        virtual bool IsActive()
        {
            return BuffTrigger::IsActive() && AI_VALUE2(bool, "swimming", "self target");
        }
    };

    class WaterBreathingTrigger : public BuffTrigger {
    public:
        WaterBreathingTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "water breathing", 5) {}

        virtual bool IsActive()
        {
            return BuffTrigger::IsActive() && AI_VALUE2(bool, "swimming", "self target");
        }
    };

    class WaterWalkingOnPartyTrigger : public BuffOnPartyTrigger {
    public:
        WaterWalkingOnPartyTrigger(PlayerbotAI* ai) : BuffOnPartyTrigger(ai, "water walking on party", 7) {}

        virtual bool IsActive()
        {
            return BuffOnPartyTrigger::IsActive() && AI_VALUE2(bool, "swimming", "self target");
        }
    };

    class WaterBreathingOnPartyTrigger : public BuffOnPartyTrigger {
    public:
        WaterBreathingOnPartyTrigger(PlayerbotAI* ai) : BuffOnPartyTrigger(ai, "water breathing on party", 2) {}

        virtual bool IsActive()
        {
            return BuffOnPartyTrigger::IsActive() && AI_VALUE2(bool, "swimming", "self target");
        }
    };

    class CleanseSpiritPoisonTrigger : public NeedCureTrigger
    {
    public:
        CleanseSpiritPoisonTrigger(PlayerbotAI* ai) : NeedCureTrigger(ai, "cleanse spirit", DISPEL_POISON) {}
    };

    class PartyMemberCleanseSpiritPoisonTrigger : public PartyMemberNeedCureTrigger
    {
    public:
        PartyMemberCleanseSpiritPoisonTrigger(PlayerbotAI* ai) : PartyMemberNeedCureTrigger(ai, "cleanse spirit", DISPEL_POISON) {}
    };

    class CleanseSpiritCurseTrigger : public NeedCureTrigger
    {
    public:
        CleanseSpiritCurseTrigger(PlayerbotAI* ai) : NeedCureTrigger(ai, "cleanse spirit", DISPEL_CURSE) {}
    };

    class PartyMemberCleanseSpiritCurseTrigger : public PartyMemberNeedCureTrigger
    {
    public:
        PartyMemberCleanseSpiritCurseTrigger(PlayerbotAI* ai) : PartyMemberNeedCureTrigger(ai, "cleanse spirit", DISPEL_CURSE) {}
    };

    class CleanseSpiritDiseaseTrigger : public NeedCureTrigger
    {
    public:
        CleanseSpiritDiseaseTrigger(PlayerbotAI* ai) : NeedCureTrigger(ai, "cleanse spirit", DISPEL_DISEASE) {}
    };

    class PartyMemberCleanseSpiritDiseaseTrigger : public PartyMemberNeedCureTrigger
    {
    public:
        PartyMemberCleanseSpiritDiseaseTrigger(PlayerbotAI* ai) : PartyMemberNeedCureTrigger(ai, "cleanse spirit", DISPEL_DISEASE) {}
    };

    class ShockTrigger : public DebuffTrigger {
    public:
        ShockTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "earth shock") {}
        virtual bool IsActive();
    };

    class FrostShockSnareTrigger : public SnareTargetTrigger {
    public:
        FrostShockSnareTrigger(PlayerbotAI* ai) : SnareTargetTrigger(ai, "frost shock") {}
    };

    class HeroismTrigger : public BoostTrigger
    {
    public:
        HeroismTrigger(PlayerbotAI* ai) : BoostTrigger(ai, "heroism") {}
    };

    class BloodlustTrigger : public BoostTrigger
    {
    public:
        BloodlustTrigger(PlayerbotAI* ai) : BoostTrigger(ai, "bloodlust") {}
    };

    class MaelstromWeaponTrigger : public HasAuraTrigger
    {
    public:
        MaelstromWeaponTrigger(PlayerbotAI* ai) : HasAuraTrigger(ai, "maelstrom weapon") {}
    };

    class WindShearInterruptEnemyHealerSpellTrigger : public InterruptEnemyHealerTrigger
    {
    public:
        WindShearInterruptEnemyHealerSpellTrigger(PlayerbotAI* ai) : InterruptEnemyHealerTrigger(ai, "wind shear") {}
    };

    class CurePoisonTrigger : public NeedCureTrigger
    {
    public:
        CurePoisonTrigger(PlayerbotAI* ai) : NeedCureTrigger(ai, "cure poison", DISPEL_POISON) {}
    };

    class PartyMemberCurePoisonTrigger : public PartyMemberNeedCureTrigger
    {
    public:
        PartyMemberCurePoisonTrigger(PlayerbotAI* ai) : PartyMemberNeedCureTrigger(ai, "cure poison", DISPEL_POISON) {}
    };

    class CureDiseaseTrigger : public NeedCureTrigger
    {
    public:
        CureDiseaseTrigger(PlayerbotAI* ai) : NeedCureTrigger(ai, "cure disease", DISPEL_DISEASE) {}
    };

    class PartyMemberCureDiseaseTrigger : public PartyMemberNeedCureTrigger
    {
    public:
        PartyMemberCureDiseaseTrigger(PlayerbotAI* ai) : PartyMemberNeedCureTrigger(ai, "cure disease", DISPEL_DISEASE) {}
    };
}
