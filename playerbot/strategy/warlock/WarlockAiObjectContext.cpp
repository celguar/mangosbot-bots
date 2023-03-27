#include "botpch.h"
#include "../../playerbot.h"
#include "WarlockActions.h"
#include "WarlockAiObjectContext.h"
#include "DpsWarlockStrategy.h"
#include "GenericWarlockNonCombatStrategy.h"
#include "WarlockReactionStrategy.h"
#include "TankWarlockStrategy.h"
#include "../generic/PullStrategy.h"
#include "WarlockTriggers.h"
#include "../NamedObjectContext.h"
#include "../actions/UseItemAction.h"

using namespace ai;

namespace ai
{
    namespace warlock
    {
        using namespace ai;

        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["nc"] = &warlock::StrategyFactoryInternal::nc;
                creators["react"] = &warlock::StrategyFactoryInternal::react;
                creators["pull"] = &warlock::StrategyFactoryInternal::pull;
                creators["aoe"] = &warlock::StrategyFactoryInternal::aoe;
                creators["dps debuff"] = &warlock::StrategyFactoryInternal::dps_debuff;
                creators["boost"] = &warlock::StrategyFactoryInternal::boost;
                creators["cc"] = &warlock::StrategyFactoryInternal::cc;
                creators["pet"] = &warlock::StrategyFactoryInternal::pet;
            }

        private:
            static Strategy* pet(PlayerbotAI* ai) { return new WarlockPetStrategy(ai); }
            static Strategy* nc(PlayerbotAI* ai) { return new GenericWarlockNonCombatStrategy(ai); }
            static Strategy* react(PlayerbotAI* ai) { return new WarlockReactionStrategy(ai); }
            static Strategy* aoe(PlayerbotAI* ai) { return new DpsAoeWarlockStrategy(ai); }
            static Strategy* dps_debuff(PlayerbotAI* ai) { return new DpsWarlockDebuffStrategy(ai); }
            static Strategy* pull(PlayerbotAI* ai) { return new PullStrategy(ai, "shoot"); }
            static Strategy* boost(PlayerbotAI* ai) { return new WarlockBoostStrategy(ai); }
            static Strategy* cc(PlayerbotAI* ai) { return new WarlockCcStrategy(ai); }

        };

        class CombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["dps"] = &warlock::CombatStrategyFactoryInternal::dps;
                creators["tank"] = &warlock::CombatStrategyFactoryInternal::tank;
            }

        private:
            static Strategy* tank(PlayerbotAI* ai) { return new TankWarlockStrategy(ai); }
            static Strategy* dps(PlayerbotAI* ai) { return new DpsWarlockStrategy(ai); }
        };
    };
};

namespace ai
{
    namespace warlock
    {
        using namespace ai;

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["shadow trance"] = &TriggerFactoryInternal::shadow_trance;
                creators["demon armor"] = &TriggerFactoryInternal::demon_armor;
                creators["no healthstone"] = &TriggerFactoryInternal::HasHealthstone;
                creators["no firestone"] = &TriggerFactoryInternal::HasFirestone;
                creators["no spellstone"] = &TriggerFactoryInternal::HasSpellstone;
                creators["corruption"] = &TriggerFactoryInternal::corruption;
                creators["corruption on attacker"] = &TriggerFactoryInternal::corruption_on_attacker;
                creators["curse of agony"] = &TriggerFactoryInternal::curse_of_agony;
                creators["curse of agony on attacker"] = &TriggerFactoryInternal::curse_of_agony_on_attacker;
                creators["banish"] = &TriggerFactoryInternal::banish;
                creators["spellstone"] = &TriggerFactoryInternal::spellstone;
                creators["backlash"] = &TriggerFactoryInternal::backlash;
                creators["fear"] = &TriggerFactoryInternal::fear;
                creators["immolate"] = &TriggerFactoryInternal::immolate;
                creators["amplify curse"] = &TriggerFactoryInternal::amplify_curse;
                creators["siphon life"] = &TriggerFactoryInternal::siphon_life;
                creators["siphon life on attacker"] = &TriggerFactoryInternal::siphon_life_on_attacker;
                creators["death coil interrupt"] = &TriggerFactoryInternal::death_coil_interrupt;
                creators["death coil on enemy healer"] = &TriggerFactoryInternal::death_coil_enemy_healer;
                creators["death coil on snare target"] = &TriggerFactoryInternal::death_coil_snare_target;
                creators["inferno"] = &TriggerFactoryInternal::inferno;
                creators["shadowfury interrupt"] = &TriggerFactoryInternal::shadowfury_interrupt;
                creators["shadowfury on snare target"] = &TriggerFactoryInternal::shadowfury_snare;
                creators["unstable affliction"] = &TriggerFactoryInternal::unstable_affliction;
            }

        private:
            static Trigger* unstable_affliction(PlayerbotAI* ai) { return new UnstableAfflictionTrigger(ai); }
            static Trigger* shadowfury_snare(PlayerbotAI* ai) { return new ShadowfurySnareTrigger(ai); }
            static Trigger* shadowfury_interrupt(PlayerbotAI* ai) { return new ShadowfuryInterruptTrigger(ai); }
            static Trigger* inferno(PlayerbotAI* ai) { return new InfernoTrigger(ai); }
            static Trigger* death_coil_snare_target(PlayerbotAI* ai) { return new DeathCoilSnareTrigger(ai); }
            static Trigger* death_coil_enemy_healer(PlayerbotAI* ai) { return new DeathCoilInterruptTHealerTrigger(ai); }
            static Trigger* death_coil_interrupt(PlayerbotAI* ai) { return new DeathCoilInterruptTrigger(ai); }
            static Trigger* amplify_curse(PlayerbotAI* ai) { return new AmplifyCurseTrigger(ai); }
            static Trigger* shadow_trance(PlayerbotAI* ai) { return new ShadowTranceTrigger(ai); }
            static Trigger* demon_armor(PlayerbotAI* ai) { return new DemonArmorTrigger(ai); }
            static Trigger* HasHealthstone(PlayerbotAI* ai) { return new HasHealthstoneTrigger(ai); }
            static Trigger* HasFirestone(PlayerbotAI* ai) { return new HasFirestoneTrigger(ai); }
            static Trigger* HasSpellstone(PlayerbotAI* ai) { return new HasSpellstoneTrigger(ai); }
            static Trigger* corruption(PlayerbotAI* ai) { return new CorruptionTrigger(ai); }
            static Trigger* corruption_on_attacker(PlayerbotAI* ai) { return new CorruptionOnAttackerTrigger(ai); }
            static Trigger* siphon_life(PlayerbotAI* ai) { return new SiphonLifeTrigger(ai); }
            static Trigger* siphon_life_on_attacker(PlayerbotAI* ai) { return new SiphonLifeOnAttackerTrigger(ai); }
            static Trigger* curse_of_agony(PlayerbotAI* ai) { return new CurseOfAgonyTrigger(ai); }
            static Trigger* curse_of_agony_on_attacker(PlayerbotAI* ai) { return new CastCurseOfAgonyOnAttackerTrigger(ai); }
            static Trigger* banish(PlayerbotAI* ai) { return new BanishTrigger(ai); }
            static Trigger* spellstone(PlayerbotAI* ai) { return new SpellstoneTrigger(ai); }
            static Trigger* backlash(PlayerbotAI* ai) { return new BacklashTrigger(ai); }
            static Trigger* fear(PlayerbotAI* ai) { return new FearTrigger(ai); }
            static Trigger* immolate(PlayerbotAI* ai) { return new ImmolateTrigger(ai); }
        };
    };
};

namespace ai
{
    namespace warlock
    {
        using namespace ai;

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["fel armor"] = &AiObjectContextInternal::fel_armor;
                creators["demon armor"] = &AiObjectContextInternal::demon_armor;
                creators["demon skin"] = &AiObjectContextInternal::demon_skin;
                creators["create healthstone"] = &AiObjectContextInternal::create_healthstone;
                creators["create firestone"] = &AiObjectContextInternal::create_firestone;
                creators["create spellstone"] = &AiObjectContextInternal::create_spellstone;
                creators["spellstone"] = &AiObjectContextInternal::spellstone;
                creators["summon voidwalker"] = &AiObjectContextInternal::summon_voidwalker;
                creators["summon succubus"] = &AiObjectContextInternal::summon_succubus;
                creators["summon felhunter"] = &AiObjectContextInternal::summon_felhunter;
                creators["summon imp"] = &AiObjectContextInternal::summon_imp;
                creators["summon felguard"] = &AiObjectContextInternal::summon_felguard;
                creators["summon inferno"] = &AiObjectContextInternal::inferno;
                creators["immolate"] = &AiObjectContextInternal::immolate;
                creators["corruption"] = &AiObjectContextInternal::corruption;
                creators["corruption on attacker"] = &AiObjectContextInternal::corruption_on_attacker;
                creators["siphon life"] = &AiObjectContextInternal::siphon_life;
                creators["siphon life on attacker"] = &AiObjectContextInternal::siphon_life_on_attacker;
                creators["curse of agony"] = &AiObjectContextInternal::curse_of_agony;
                creators["curse of agony on attacker"] = &AiObjectContextInternal::curse_of_agony_on_attacker;
                creators["shadow bolt"] = &AiObjectContextInternal::shadow_bolt;
                creators["drain soul"] = &AiObjectContextInternal::drain_soul;
                creators["drain mana"] = &AiObjectContextInternal::drain_mana;
                creators["drain life"] = &AiObjectContextInternal::drain_life;
                creators["banish"] = &AiObjectContextInternal::banish;
                creators["banish on cc"] = &AiObjectContextInternal::banish_on_cc;
                creators["seed of corruption"] = &AiObjectContextInternal::seed_of_corruption;
                creators["rain of fire"] = &AiObjectContextInternal::rain_of_fire;
                creators["shadowfury"] = &AiObjectContextInternal::shadowfury;
                creators["shadowfury on snare target"] = &AiObjectContextInternal::shadowfury_snare;
                creators["life tap"] = &AiObjectContextInternal::life_tap;
                creators["fear"] = &AiObjectContextInternal::fear;
                creators["fear on cc"] = &AiObjectContextInternal::fear_on_cc;
                creators["incinerate"] = &AiObjectContextInternal::incinerate;
                creators["conflagrate"] = &AiObjectContextInternal::conflagrate;
                creators["amplify curse"] = &AiObjectContextInternal::amplify_curse;
                creators["shadowburn"] = &AiObjectContextInternal::shadowburn;
                creators["death coil"] = &AiObjectContextInternal::death_coil;
                creators["death coil on enemy healer"] = &AiObjectContextInternal::death_coil_healer;
                creators["death coil on snare target"] = &AiObjectContextInternal::death_coil_snare;
                creators["dark pact"] = &AiObjectContextInternal::dark_pact;
                creators["howl of terror"] = &AiObjectContextInternal::howl_of_terror;
                creators["unstable affliction"] = &AiObjectContextInternal::unstable_affliction;
                creators["soul shatter"] = &AiObjectContextInternal::soul_shatter;
                creators["searing pain"] = &AiObjectContextInternal::searing_pain;
            }

        private:
            static Action* searing_pain(PlayerbotAI* ai) { return new CastSearingPainAction(ai); }
            static Action* soul_shatter(PlayerbotAI* ai) { return new CastSoulShatterAction(ai); }
            static Action* unstable_affliction(PlayerbotAI* ai) { return new CastUnstableAfflictionAction(ai); }
            static Action* howl_of_terror(PlayerbotAI* ai) { return new CastHowlOfTerrorAction(ai); }
            static Action* dark_pact(PlayerbotAI* ai) { return new CastDarkPactAction(ai); }
            static Action* death_coil_snare(PlayerbotAI* ai) { return new CastDeathCoilSnareAction(ai); }
            static Action* death_coil_healer(PlayerbotAI* ai) { return new CastDeathCoilOnHealerAction(ai); }
            static Action* death_coil(PlayerbotAI* ai) { return new CastDeathCoilAction(ai); }
            static Action* amplify_curse(PlayerbotAI* ai) { return new CastAmplifyCurseAction(ai); }
            static Action* shadowburn(PlayerbotAI* ai) { return new CastShadowburnAction(ai); }
            static Action* conflagrate(PlayerbotAI* ai) { return new CastConflagrateAction(ai); }
            static Action* incinerate(PlayerbotAI* ai) { return new CastIncinerateAction(ai); }
            static Action* fear_on_cc(PlayerbotAI* ai) { return new CastFearOnCcAction(ai); }
            static Action* fear(PlayerbotAI* ai) { return new CastFearAction(ai); }
            static Action* immolate(PlayerbotAI* ai) { return new CastImmolateAction(ai); }
            static Action* summon_imp(PlayerbotAI* ai) { return new CastSummonImpAction(ai); }
            static Action* summon_succubus(PlayerbotAI* ai) { return new CastSummonSuccubusAction(ai); }
            static Action* summon_felhunter(PlayerbotAI * ai) { return new CastSummonFelhunterAction(ai); }
            static Action* fel_armor(PlayerbotAI* ai) { return new CastFelArmorAction(ai); }
            static Action* demon_armor(PlayerbotAI* ai) { return new CastDemonArmorAction(ai); }
            static Action* demon_skin(PlayerbotAI* ai) { return new CastDemonSkinAction(ai); }
            static Action* create_healthstone(PlayerbotAI* ai) { return new CastCreateHealthstoneAction(ai); }
            static Action* create_firestone(PlayerbotAI* ai) { return new CastCreateFirestoneAction(ai); }
            static Action* create_spellstone(PlayerbotAI* ai) { return new CastCreateSpellstoneAction(ai); }
            static Action* spellstone(PlayerbotAI* ai) { return new UseSpellItemAction(ai, "spellstone", true); }
            static Action* summon_voidwalker(PlayerbotAI* ai) { return new CastSummonVoidwalkerAction(ai); }
            static Action* summon_felguard(PlayerbotAI* ai) { return new CastSummonFelguardAction(ai); }
            static Action* inferno(PlayerbotAI* ai) { return new CastSummonInfernoAction(ai); }
            static Action* corruption(PlayerbotAI* ai) { return new CastCorruptionAction(ai); }
            static Action* corruption_on_attacker(PlayerbotAI* ai) { return new CastCorruptionOnAttackerAction(ai); }
            static Action* siphon_life(PlayerbotAI* ai) { return new CastSiphonLifeAction(ai); }
            static Action* siphon_life_on_attacker(PlayerbotAI* ai) { return new CastSiphonLifeOnAttackerAction(ai); }
            static Action* curse_of_agony(PlayerbotAI* ai) { return new CastCurseOfAgonyAction(ai); }
            static Action* curse_of_agony_on_attacker(PlayerbotAI* ai) { return new CastCurseOfAgonyOnAttackerAction(ai); }
            static Action* shadow_bolt(PlayerbotAI* ai) { return new CastShadowBoltAction(ai); }
            static Action* drain_soul(PlayerbotAI* ai) { return new CastDrainSoulAction(ai); }
            static Action* drain_mana(PlayerbotAI* ai) { return new CastDrainManaAction(ai); }
            static Action* drain_life(PlayerbotAI* ai) { return new CastDrainLifeAction(ai); }
            static Action* banish(PlayerbotAI* ai) { return new CastBanishAction(ai); }
            static Action* banish_on_cc(PlayerbotAI* ai) { return new CastBanishAction(ai); }
            static Action* seed_of_corruption(PlayerbotAI* ai) { return new CastSeedOfCorruptionAction(ai); }
            static Action* rain_of_fire(PlayerbotAI* ai) { return new CastRainOfFireAction(ai); }
            static Action* shadowfury(PlayerbotAI* ai) { return new CastShadowfuryAction(ai); }
            static Action* shadowfury_snare(PlayerbotAI* ai) { return new CastShadowfurySnareAction(ai); }
            static Action* life_tap(PlayerbotAI* ai) { return new CastLifeTapAction(ai); }

        };
    };
};



WarlockAiObjectContext::WarlockAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::warlock::StrategyFactoryInternal());
    strategyContexts.Add(new ai::warlock::CombatStrategyFactoryInternal());
    actionContexts.Add(new ai::warlock::AiObjectContextInternal());
    triggerContexts.Add(new ai::warlock::TriggerFactoryInternal());
}
