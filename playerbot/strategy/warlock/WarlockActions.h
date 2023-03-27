#pragma once

#include "../actions/GenericActions.h"

namespace ai
{
	SNARE_ACTION(CastDeathCoilSnareAction, "death coil");
	ENEMY_HEALER_ACTION(CastDeathCoilOnHealerAction, "death coil");
	SPELL_ACTION(CastDeathCoilAction, "death coil");
	
	class CastDemonSkinAction : public CastBuffSpellAction 
	{
	public:
		CastDemonSkinAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "demon skin") {}
	};

	class CastDemonArmorAction : public CastBuffSpellAction
	{
	public:
		CastDemonArmorAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "demon armor") {}
	};

	class CastFelArmorAction : public CastBuffSpellAction
	{
	public:
		CastFelArmorAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "fel armor") {}
	};

    BEGIN_RANGED_SPELL_ACTION(CastShadowBoltAction, "shadow bolt")
    END_SPELL_ACTION()

	class CastDrainSoulAction : public CastSpellAction
	{
	public:
		CastDrainSoulAction(PlayerbotAI* ai) : CastSpellAction(ai, "drain soul") {}
		virtual bool isUseful()
		{
			return AI_VALUE2(uint32, "item count", "soul shard") < uint32(AI_VALUE(uint8, "bag space") * 0.2);
		}
	};

	SPELL_ACTION(CastShadowburnAction, "shadowburn");
	BUFF_ACTION(CastDarkPactAction, "dark pact");
	BUFF_ACTION(CastSoulShatterAction, "soul shatter");

	class CastDrainManaAction : public CastSpellAction
	{
	public:
		CastDrainManaAction(PlayerbotAI* ai) : CastSpellAction(ai, "drain mana") {}
	};

	class CastDrainLifeAction : public CastSpellAction
	{
	public:
		CastDrainLifeAction(PlayerbotAI* ai) : CastSpellAction(ai, "drain life") {}
	};

	class CastCurseOfAgonyAction : public CastRangedDebuffSpellAction
	{
	public:
		CastCurseOfAgonyAction(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, "curse of agony") {}
	};

	class CastCurseOfWeaknessAction : public CastRangedDebuffSpellAction
	{
	public:
		CastCurseOfWeaknessAction(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, "curse of weakness") {}
	};

	class CastCorruptionAction : public CastRangedDebuffSpellAction
	{
	public:
		CastCorruptionAction(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, "corruption") {}
	};

	class CastCorruptionOnAttackerAction : public CastRangedDebuffSpellOnAttackerAction
	{
	public:
	    CastCorruptionOnAttackerAction(PlayerbotAI* ai) : CastRangedDebuffSpellOnAttackerAction(ai, "corruption") {}
	};

    class CastCurseOfAgonyOnAttackerAction : public CastRangedDebuffSpellOnAttackerAction
    {
    public:
        CastCurseOfAgonyOnAttackerAction(PlayerbotAI* ai) : CastRangedDebuffSpellOnAttackerAction(ai, "curse of agony") {}
    };

	class CastSummonVoidwalkerAction : public CastBuffSpellAction
	{
	public:
		CastSummonVoidwalkerAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "summon voidwalker") {}
	};

	class CastSummonFelguardAction : public CastBuffSpellAction
	{
	public:
		CastSummonFelguardAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "summon felguard") {}
	};

	class CastSummonImpAction : public CastBuffSpellAction
	{
	public:
		CastSummonImpAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "summon imp") {}
	};

    class CastSummonSuccubusAction : public CastBuffSpellAction
    {
    public:
        CastSummonSuccubusAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "summon succubus") {}
    };

	class CastSummonFelhunterAction : public CastBuffSpellAction
	{
	public:
		CastSummonFelhunterAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "summon felhunter") {}
	};

	class CastSummonInfernoAction : public CastSpellAction
	{
	public:
		CastSummonInfernoAction(PlayerbotAI* ai) : CastSpellAction(ai, "inferno") {}
		virtual bool isPossible() { return true; }
	};

	class CastCreateHealthstoneAction : public CastBuffSpellAction
	{
	public:
		CastCreateHealthstoneAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "create healthstone") {}
	};

	class CastCreateFirestoneAction : public CastBuffSpellAction
	{
	public:
		CastCreateFirestoneAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "create firestone") {}
	};

	class CastCreateSpellstoneAction : public CastBuffSpellAction
	{
	public:
		CastCreateSpellstoneAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "create spellstone") {}
	};

    class CastBanishAction : public CastCrowdControlSpellAction
    {
    public:
        CastBanishAction(PlayerbotAI* ai) : CastCrowdControlSpellAction(ai, "banish") {}
    };

    class CastSeedOfCorruptionAction : public CastRangedDebuffSpellAction
    {
    public:
        CastSeedOfCorruptionAction(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, "seed of corruption") {}
    };

    class CastRainOfFireAction : public CastSpellAction
    {
    public:
        CastRainOfFireAction(PlayerbotAI* ai) : CastSpellAction(ai, "rain of fire") {}
    };

    class CastShadowfuryAction : public CastSpellAction
    {
    public:
        CastShadowfuryAction(PlayerbotAI* ai) : CastSpellAction(ai, "shadowfury") {}
    };

	SNARE_ACTION(CastShadowfurySnareAction, "shadowfury");
	RANGED_DEBUFF_ACTION(CastUnstableAfflictionAction, "unstable affliction");

    class CastImmolateAction : public CastRangedDebuffSpellAction
    {
    public:
        CastImmolateAction(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, "immolate") {}
    };

    class CastConflagrateAction : public CastSpellAction
    {
    public:
        CastConflagrateAction(PlayerbotAI* ai) : CastSpellAction(ai, "conflagrate") {}
    };

    class CastIncinerateAction : public CastSpellAction
    {
    public:
        CastIncinerateAction(PlayerbotAI* ai) : CastSpellAction(ai, "incinerate") {}
    };

    class CastFearAction : public CastRangedDebuffSpellAction
    {
    public:
        CastFearAction(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, "fear") {}
    };

    class CastFearOnCcAction : public CastCrowdControlSpellAction
    {
    public:
        CastFearOnCcAction(PlayerbotAI* ai) : CastCrowdControlSpellAction(ai, "fear") {}
    };

    class CastLifeTapAction: public CastSpellAction
    {
    public:
        CastLifeTapAction(PlayerbotAI* ai) : CastSpellAction(ai, "life tap") {}
        virtual string GetTargetName() { return "self target"; }
        virtual bool isUseful() { return AI_VALUE2(uint8, "health", "self target") > sPlayerbotAIConfig.lowHealth; }
    };

    class CastAmplifyCurseAction : public CastBuffSpellAction
    {
    public:
        CastAmplifyCurseAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "amplify curse") {}
    };

    class CastSiphonLifeAction : public CastRangedDebuffSpellAction
    {
    public:
        CastSiphonLifeAction(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, "siphon life") {}
    };

    class CastSiphonLifeOnAttackerAction : public CastRangedDebuffSpellOnAttackerAction
    {
    public:
        CastSiphonLifeOnAttackerAction(PlayerbotAI* ai) : CastRangedDebuffSpellOnAttackerAction(ai, "siphon life") {}
    };

    class CastHowlOfTerrorAction : public CastMeleeAoeSpellAction
    {
    public:
		CastHowlOfTerrorAction(PlayerbotAI* ai) : CastMeleeAoeSpellAction(ai, "howl of terror", 10.0f) {}
    };

	SPELL_ACTION(CastSearingPainAction, "searing pain");
}
