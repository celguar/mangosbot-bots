#pragma once
#include "../triggers/GenericTriggers.h"

namespace ai {

    class MarkOfTheWildOnPartyTrigger : public BuffOnPartyTrigger {
    public:
        MarkOfTheWildOnPartyTrigger(PlayerbotAI* ai) : BuffOnPartyTrigger(ai, "mark of the wild", 4) {}

        virtual bool IsActive() { return BuffOnPartyTrigger::IsActive() && !ai->HasAura("mark of the wild", GetTarget()) && !ai->HasAura("gift of the wild", GetTarget()); }
    };

    class MarkOfTheWildTrigger : public BuffTrigger {
    public:
        MarkOfTheWildTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "mark of the wild", 4) {}

        virtual bool IsActive() { return BuffTrigger::IsActive() && !ai->HasAura("mark of the wild", GetTarget()) && !ai->HasAura("gift of the wild", GetTarget()); }
    };

    class GiftOfTheWildTrigger : public BuffOnPartyTrigger {
    public:
        GiftOfTheWildTrigger(PlayerbotAI* ai) : BuffOnPartyTrigger(ai, "gift of the wild", 3) {}

        virtual bool IsActive() {
            return bot->GetGroup() && BuffOnPartyTrigger::IsActive() &&
                !ai->HasAura("gift of the wild", GetTarget()) &&
#ifdef MANGOS
                (ai->GetBot()->IsInSameGroupWith((Player*)GetTarget()) || ai->GetBot()->IsInSameRaidWith((Player*)GetTarget())) &&
#endif
#ifdef CMANGOS
                bot->IsInGroup((Player*)GetTarget()) &&
#endif
                (ai->GetBuffedCount((Player*)GetTarget(), "gift of the wild") + ai->GetBuffedCount((Player*)GetTarget(), "mark of the wild")) < 4;
            ;
        }
    };

    class ThornsOnPartyTrigger : public BuffOnPartyTrigger
    {
    public:
        ThornsOnPartyTrigger(PlayerbotAI* ai) : BuffOnPartyTrigger(ai, "thorns", 2) {}

        virtual bool IsActive()
        {
            Unit* target = GetTarget();
            return BuffOnPartyTrigger::IsActive() && (!target->IsPlayer() || !ai->IsRanged((Player*)target));
        }
    };

    class ThornsTrigger : public BuffTrigger
    {
    public:
        ThornsTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "thorns", 2) {}

        virtual bool IsActive() { return BuffTrigger::IsActive() && !ai->HasAura("thorns", GetTarget()); }
    };

    class OmenOfClarityTrigger : public BuffTrigger
    {
    public:
        OmenOfClarityTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "omen of clarity") {}
    };

    class RakeTrigger : public DebuffTrigger
    {
    public:
        RakeTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "rake") {}
    };

    class InsectSwarmTrigger : public DebuffTrigger
    {
    public:
        InsectSwarmTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "insect swarm") {}
    };

    class MoonfireTrigger : public DebuffTrigger
    {
    public:
        MoonfireTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "moonfire") {}

        virtual bool IsActive() { return DebuffTrigger::IsActive() && !GetTarget()->IsRooted(); }
    };

    class FaerieFireTrigger : public DebuffTrigger
    {
    public:
        FaerieFireTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "faerie fire") {}
    };

    class FaerieFireFeralTrigger : public DebuffTrigger
    {
    public:
        FaerieFireFeralTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "faerie fire (feral)") {}
    };

    class BashInterruptSpellTrigger : public InterruptSpellTrigger
    {
    public:
        BashInterruptSpellTrigger(PlayerbotAI* ai) : InterruptSpellTrigger(ai, "bash") {}
    };

    class TigersFuryTrigger : public BoostTrigger
    {
    public:
        TigersFuryTrigger(PlayerbotAI* ai) : BoostTrigger(ai, "tiger's fury") {}
    };

    class NaturesGraspTrigger : public BoostTrigger
    {
    public:
        NaturesGraspTrigger(PlayerbotAI* ai) : BoostTrigger(ai, "nature's grasp") {}
    };

    class EntanglingRootsTrigger : public HasCcTargetTrigger
    {
    public:
        EntanglingRootsTrigger(PlayerbotAI* ai) : HasCcTargetTrigger(ai, "entangling roots") {}
    };

    class EntanglingRootsKiteTrigger : public DebuffTrigger
    {
    public:
        EntanglingRootsKiteTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "entangling roots") {}

        virtual bool IsActive() { return DebuffTrigger::IsActive() && AI_VALUE(uint8, "attacker count") < 3 && !GetTarget()->HasMana(); }
    };

    class EntanglingRootsSnareTrigger : public SnareTargetTrigger
    {
    public:
        EntanglingRootsSnareTrigger(PlayerbotAI* ai) : SnareTargetTrigger(ai, "entangling roots", 5) {}
    };

    class HibernateTrigger : public HasCcTargetTrigger
    {
    public:
        HibernateTrigger(PlayerbotAI* ai) : HasCcTargetTrigger(ai, "hibernate") {}
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

    CURE_TRIGGER(RemoveCurseTrigger, "remove curse", DISPEL_CURSE);
    CURE_PARTY_TRIGGER(RemoveCurseOnPartyTrigger, "remove curse", DISPEL_CURSE);

    class BearFormTrigger : public BuffTrigger
    {
    public:
        BearFormTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "bear form") {}
        virtual bool IsActive() { return !ai->HasAnyAuraOf(bot, "bear form", "dire bear form", NULL); }
    };

    class TreeFormTrigger : public BuffTrigger
    {
    public:
        TreeFormTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "tree of life") {}
        virtual bool IsActive() { return !ai->HasAura("tree of life", bot); }
    };

    class CatFormTrigger : public BuffTrigger
    {
    public:
        CatFormTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "cat form") {}
        virtual bool IsActive() { return !ai->HasAura("cat form", bot); }
    };

    class EclipseSolarTrigger : public HasAuraTrigger
    {
    public:
        EclipseSolarTrigger(PlayerbotAI* ai) : HasAuraTrigger(ai, "eclipse (solar)") {}
    };

    class EclipseLunarTrigger : public HasAuraTrigger
    {
    public:
        EclipseLunarTrigger(PlayerbotAI* ai) : HasAuraTrigger(ai, "eclipse (lunar)") {}
    };

    class BashInterruptEnemyHealerSpellTrigger : public InterruptEnemyHealerTrigger
    {
    public:
        BashInterruptEnemyHealerSpellTrigger(PlayerbotAI* ai) : InterruptEnemyHealerTrigger(ai, "bash") {}
    };

    class NaturesSwiftnessTrigger : public BuffTrigger
    {
    public:
        NaturesSwiftnessTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "nature's swiftness") {}
    };
}
