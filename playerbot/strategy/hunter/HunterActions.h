#pragma once

#include "../actions/GenericActions.h"

namespace ai
{
    BEGIN_RANGED_DEBUFF_ACTION(CastHuntersMarkAction, "hunter's mark")
    END_SPELL_ACTION()

    class CastAutoShotAction : public CastSpellAction
    {
    public:
        CastAutoShotAction(PlayerbotAI* ai) : CastSpellAction(ai, "auto shot") {}
        virtual bool isUseful();
    };

    BEGIN_RANGED_SPELL_ACTION(CastTranquilizingShotAction, "tranquilizing shot")
    END_SPELL_ACTION()

    BEGIN_RANGED_SPELL_ACTION(CastArcaneShotAction, "arcane shot")
    END_SPELL_ACTION()

    BEGIN_RANGED_SPELL_ACTION(CastExplosiveShotAction, "explosive shot")
    END_SPELL_ACTION()

    BEGIN_RANGED_SPELL_ACTION(CastAimedShotAction, "aimed shot")
    END_SPELL_ACTION()

    BEGIN_RANGED_SPELL_ACTION(CastChimeraShotAction, "chimera shot")
    END_SPELL_ACTION()

    BEGIN_RANGED_DEBUFF_ACTION(CastConcussiveShotAction, "concussive shot")
    END_SPELL_ACTION()

    SPELL_ACTION(CastSteadyShotAction, "steady shot");

    BEGIN_RANGED_DEBUFF_ACTION(CastScatterShotAction, "scatter shot")
    END_SPELL_ACTION()

    BEGIN_RANGED_SPELL_ACTION(CastDistractingShotAction, "distracting shot")
    END_SPELL_ACTION()

    BEGIN_RANGED_SPELL_ACTION(CastMultiShotAction, "multi-shot")
    END_SPELL_ACTION()

    BEGIN_RANGED_SPELL_ACTION(CastVolleyAction, "volley")
    END_SPELL_ACTION()

    BEGIN_RANGED_DEBUFF_ACTION(CastSerpentStingAction, "serpent sting")
        virtual bool isUseful();
    END_SPELL_ACTION()

    BEGIN_RANGED_DEBUFF_ACTION(CastViperStingAction, "viper sting")
        virtual bool isUseful();
    END_SPELL_ACTION()

    BEGIN_RANGED_SPELL_ACTION(CastScorpidStingAction, "scorpid sting")
    END_SPELL_ACTION()

    class CastAspectOfTheMonkeyAction : public CastBuffSpellAction
    {
    public:
        CastAspectOfTheMonkeyAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "aspect of the monkey") {}
    };

    class CastAspectOfTheHawkAction : public CastBuffSpellAction
    {
    public:
        CastAspectOfTheHawkAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "aspect of the hawk") {}
    };

    class CastAspectOfTheWildAction : public CastBuffSpellAction
    {
    public:
        CastAspectOfTheWildAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "aspect of the wild") {}
    };

    class CastAspectOfTheCheetahAction : public CastBuffSpellAction
    {
    public:
        CastAspectOfTheCheetahAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "aspect of the cheetah") {}
        virtual bool isUseful();
    };

    class CastAspectOfThePackAction : public CastBuffSpellAction
    {
    public:
        CastAspectOfThePackAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "aspect of the pack") {}
    };

    class CastAspectOfTheViperAction : public CastBuffSpellAction
    {
    public:
        CastAspectOfTheViperAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "aspect of the viper") {}
    };

    class CastCallPetAction : public CastBuffSpellAction
    {
    public:
        CastCallPetAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "call pet") {}
    };

    class CastMendPetAction : public CastAuraSpellAction
    {
    public:
        CastMendPetAction(PlayerbotAI* ai) : CastAuraSpellAction(ai, "mend pet") {}
        virtual string GetTargetName() { return "pet target"; }
    };

    class CastRevivePetAction : public CastBuffSpellAction
    {
    public:
        CastRevivePetAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "revive pet") {}
    };

    class CastTrueshotAuraAction : public CastBuffSpellAction
    {
    public:
        CastTrueshotAuraAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "trueshot aura") {}
    };

    class CastFeignDeathAction : public CastBuffSpellAction
    {
    public:
        CastFeignDeathAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "feign death") {}
    };

    REMOVE_BUFF_ACTION(RemoveFeignDeathAction, "feign death");

    class CastRapidFireAction : public CastBuffSpellAction
    {
    public:
        CastRapidFireAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "rapid fire") {}
    };

    class CastBlackArrow : public CastRangedDebuffSpellAction
    {
    public:
        CastBlackArrow(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, "black arrow") {}
    };

    BUFF_ACTION(CastFreezingTrapAction, "freezing trap");
    BUFF_ACTION(CastFrostTrapAction, "frost trap");
    BUFF_ACTION(CastExplosiveTrapAction, "explosive trap");
    SNARE_ACTION(CastBlackArrowSnareAction, "black arrow");
    SPELL_ACTION(CastSilencingShotAction, "silencing shot");
    ENEMY_HEALER_ACTION(CastSilencingShotOnHealerAction, "silencing shot");
    BUFF_ACTION(CastReadinessAction, "readiness");
    ;
    class CastWingClipAction : public CastMeleeSpellAction
    {
    public:
        CastWingClipAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "wing clip") {}

        virtual bool isUseful()
        {
            return CastMeleeSpellAction::isUseful() && !ai->HasAura(GetSpellName(), GetTarget());
        }
    };

    class CastRaptorStrikeAction : public CastMeleeSpellAction
    {
    public:
        CastRaptorStrikeAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "raptor strike") {}

        virtual bool isUseful()
        {
            Unit* target = GetTarget();
            return CastMeleeSpellAction::isUseful() && (ai->HasStrategy("close", BotState::BOT_STATE_COMBAT) || bot->hasUnitState(UNIT_STAT_MELEE_ATTACKING));
        }
    };

    class CastSerpentStingOnAttackerAction : public CastRangedDebuffSpellOnAttackerAction
    {
    public:
        CastSerpentStingOnAttackerAction(PlayerbotAI* ai) : CastRangedDebuffSpellOnAttackerAction(ai, "serpent sting") {}
    };

    class CastViperStingOnAttackerAction : public CastRangedDebuffSpellOnAttackerAction
    {
    public:
        CastViperStingOnAttackerAction(PlayerbotAI* ai) : CastRangedDebuffSpellOnAttackerAction(ai, "viper sting") {}
    };

    class FeedPetAction : public Action
    {
    public:
        FeedPetAction(PlayerbotAI* ai) : Action(ai, "feed pet") {}
        virtual bool Execute(Event& event);
    };

    class CastBestialWrathAction : public CastBuffSpellAction
    {
    public:
        CastBestialWrathAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "bestial wrath") {}
    };

    class CastScareBeastAction : public CastSpellAction
    {
    public:
        CastScareBeastAction(PlayerbotAI* ai) : CastSpellAction(ai, "scare beast") {}
    };

    class CastScareBeastCcAction : public CastSpellAction
    {
    public:
        CastScareBeastCcAction(PlayerbotAI* ai) : CastSpellAction(ai, "scare beast on cc") {}
        virtual Value<Unit*>* GetTargetValue();
        virtual bool Execute(Event& event);
    };

    BUFF_ACTION(IntimidationAction, "intimidation");
    BUFF_ACTION(DeterrenceAction, "deterrence");
    MELEE_ACTION(CastCounterattackAction, "counterattack");
    SNARE_ACTION(WyvernStingSnareAction, "wyvern sting");
    MELEE_ACTION(MongooseBiteAction, "mongoose bite");

    class TameBeastAction : public CastSpellAction
    {
    public:
        TameBeastAction(PlayerbotAI* ai) : CastSpellAction(ai, "tame beast") {}
    };
}
