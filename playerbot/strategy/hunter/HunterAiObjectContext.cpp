#include "botpch.h"
#include "../../playerbot.h"
#include "HunterActions.h"
#include "HunterTriggers.h"
#include "HunterAiObjectContext.h"
#include "DpsHunterStrategy.h"
#include "GenericHunterNonCombatStrategy.h"
#include "HunterReactionStrategy.h"
#include "HunterBuffStrategies.h"
#include "../NamedObjectContext.h"

using namespace ai;

namespace ai
{
    namespace hunter
    {
        using namespace ai;

        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["dps"] = &hunter::StrategyFactoryInternal::dps;
                creators["nc"] = &hunter::StrategyFactoryInternal::nc;
                creators["react"] = &hunter::StrategyFactoryInternal::react;
                creators["pull"] = &hunter::StrategyFactoryInternal::pull;
                creators["aoe"] = &hunter::StrategyFactoryInternal::aoe;
                creators["dps debuff"] = &hunter::StrategyFactoryInternal::dps_debuff;
                creators["boost"] = &hunter::StrategyFactoryInternal::boost;
                creators["pet"] = &hunter::StrategyFactoryInternal::pet;
                creators["cc"] = &hunter::StrategyFactoryInternal::cc;
            }

        private:
            static Strategy* aoe(PlayerbotAI* ai) { return new DpsAoeHunterStrategy(ai); }
            static Strategy* dps(PlayerbotAI* ai) { return new DpsHunterStrategy(ai); }
            static Strategy* nc(PlayerbotAI* ai) { return new GenericHunterNonCombatStrategy(ai); }
            static Strategy* react(PlayerbotAI* ai) { return new HunterReactionStrategy(ai); }
            static Strategy* pull(PlayerbotAI* ai) { return new PullStrategy(ai, "serpent sting"); }
            static Strategy* dps_debuff(PlayerbotAI* ai) { return new DpsHunterDebuffStrategy(ai); }
            static Strategy* boost(PlayerbotAI* ai) { return new HunterBoostStrategy(ai); }
            static Strategy* pet(PlayerbotAI* ai) { return new HunterPetStrategy(ai); }
            static Strategy* cc(PlayerbotAI* ai) { return new HunterCcStrategy(ai); }
        };

        class BuffStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BuffStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["bspeed"] = &hunter::BuffStrategyFactoryInternal::bspeed;
                creators["bdps"] = &hunter::BuffStrategyFactoryInternal::bdps;
                creators["bmana"] = &hunter::BuffStrategyFactoryInternal::bmana;
                creators["rnature"] = &hunter::BuffStrategyFactoryInternal::rnature;
            }

        private:
            static Strategy* bspeed(PlayerbotAI* ai) { return new HunterBuffSpeedStrategy(ai); }
            static Strategy* bdps(PlayerbotAI* ai) { return new HunterBuffDpsStrategy(ai); }
            static Strategy* bmana(PlayerbotAI* ai) { return new HunterBuffManaStrategy(ai); }
            static Strategy* rnature(PlayerbotAI* ai) { return new HunterNatureResistanceStrategy(ai); }
        };
    };
};

namespace ai
{
    namespace hunter
    {
        using namespace ai;

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["aspect of the viper"] = &TriggerFactoryInternal::aspect_of_the_viper;
                creators["black arrow"] = &TriggerFactoryInternal::black_arrow;
                creators["black arrow on snare target"] = &TriggerFactoryInternal::black_arrow_snare;
                creators["no stings"] = &TriggerFactoryInternal::NoStings;
                creators["hunters pet dead"] = &TriggerFactoryInternal::hunters_pet_dead;
                creators["hunters pet low health"] = &TriggerFactoryInternal::hunters_pet_low_health;
                creators["hunter's mark"] = &TriggerFactoryInternal::hunters_mark;
                creators["freezing trap"] = &TriggerFactoryInternal::freezing_trap;
                creators["aspect of the pack"] = &TriggerFactoryInternal::aspect_of_the_pack;
                creators["rapid fire"] = &TriggerFactoryInternal::rapid_fire;
                creators["aspect of the hawk"] = &TriggerFactoryInternal::aspect_of_the_hawk;
                creators["aspect of the wild"] = &TriggerFactoryInternal::aspect_of_the_wild;
                creators["aspect of the viper"] = &TriggerFactoryInternal::aspect_of_the_viper;
                creators["trueshot aura"] = &TriggerFactoryInternal::trueshot_aura;
                creators["serpent sting on attacker"] = &TriggerFactoryInternal::serpent_sting_on_attacker;
                creators["viper sting on attacker"] = &TriggerFactoryInternal::viper_sting_on_attacker;
                creators["pet not happy"] = &TriggerFactoryInternal::pet_not_happy;
                creators["concussive shot on snare target"] = &TriggerFactoryInternal::concussive_shot_on_snare_target;
                creators["scare beast"] = &TriggerFactoryInternal::scare_beast;
                creators["low ammo"] = &TriggerFactoryInternal::low_ammo;
                creators["no ammo"] = &TriggerFactoryInternal::no_ammo;
                creators["has ammo"] = &TriggerFactoryInternal::has_ammo;
                creators["switch to melee"] = &TriggerFactoryInternal::switch_to_melee;
                creators["switch to ranged"] = &TriggerFactoryInternal::switch_to_ranged;
                creators["feign death"] = &TriggerFactoryInternal::feign_death;
                creators["scatter shot on snare target"] = &TriggerFactoryInternal::scatter_shot;
                creators["multi shot"] = &TriggerFactoryInternal::multi_shot;
                creators["intimidation on snare target"] = &TriggerFactoryInternal::intimidation;
                creators["counterattack"] = &TriggerFactoryInternal::counterattack;
                creators["wyvern sting"] = &TriggerFactoryInternal::wyvern_sting;
                creators["mongoose bite"] = &TriggerFactoryInternal::mongoose_bite;
                creators["viper sting"] = &TriggerFactoryInternal::viper_sting;
                creators["aimed shot"] = &TriggerFactoryInternal::aimed_shot;
                creators["bestial wrath"] = &TriggerFactoryInternal::bestial_wrath;
                creators["silencing shot interrupt"] = &TriggerFactoryInternal::silencing_shot_interrupt;
                creators["silencing shot on enemy healer"] = &TriggerFactoryInternal::silencing_shot_interrupt_healer;
                creators["no beast"] = &TriggerFactoryInternal::no_beast;
            }

        private:
            static Trigger* silencing_shot_interrupt_healer(PlayerbotAI* ai) { return new SilencingShotInterruptHealerTrigger(ai); }
            static Trigger* silencing_shot_interrupt(PlayerbotAI* ai) { return new SilencingShotInterruptTrigger(ai); }
            static Trigger* bestial_wrath(PlayerbotAI* ai) { return new BestialWrathBoostTrigger(ai); }
            static Trigger* aimed_shot(PlayerbotAI* ai) { return new AimedShotTrigger(ai); }
            static Trigger* viper_sting(PlayerbotAI* ai) { return new ViperStingTrigger(ai); }
            static Trigger* black_arrow_snare(PlayerbotAI* ai) { return new BlackArrowSnareTrigger(ai); }
            static Trigger* mongoose_bite(PlayerbotAI* ai) { return new MongooseBiteCastTrigger(ai); }
            static Trigger* wyvern_sting(PlayerbotAI* ai) { return new WybernStingSnareTrigger(ai); }
            static Trigger* counterattack(PlayerbotAI* ai) { return new CounterattackCanCastTrigger(ai); }
            static Trigger* intimidation(PlayerbotAI* ai) { return new IntimidationSnareTrigger(ai); }
            static Trigger* multi_shot(PlayerbotAI* ai) { return new MultishotCanCastTrigger(ai); }
            static Trigger* scatter_shot(PlayerbotAI* ai) { return new ScatterShotSnareTrigger(ai); }
            static Trigger* scare_beast(PlayerbotAI* ai) { return new ScareBeastTrigger(ai); }
            static Trigger* concussive_shot_on_snare_target(PlayerbotAI* ai) { return new ConsussiveShotSnareTrigger(ai); }
            static Trigger* pet_not_happy(PlayerbotAI* ai) { return new HunterPetNotHappy(ai); }
            static Trigger* serpent_sting_on_attacker(PlayerbotAI* ai) { return new SerpentStingOnAttackerTrigger(ai); }
            static Trigger* viper_sting_on_attacker(PlayerbotAI* ai) { return new ViperStingOnAttackerTrigger(ai); }
            static Trigger* trueshot_aura(PlayerbotAI* ai) { return new TrueshotAuraTrigger(ai); }
            static Trigger* aspect_of_the_viper(PlayerbotAI* ai) { return new HunterAspectOfTheViperTrigger(ai); }
            static Trigger* black_arrow(PlayerbotAI* ai) { return new BlackArrowTrigger(ai); }
            static Trigger* NoStings(PlayerbotAI* ai) { return new HunterNoStingsActiveTrigger(ai); }
            static Trigger* hunters_pet_dead(PlayerbotAI* ai) { return new HuntersPetDeadTrigger(ai); }
            static Trigger* hunters_pet_low_health(PlayerbotAI* ai) { return new HuntersPetLowHealthTrigger(ai); }
            static Trigger* hunters_mark(PlayerbotAI* ai) { return new HuntersMarkTrigger(ai); }
            static Trigger* freezing_trap(PlayerbotAI* ai) { return new FreezingTrapTrigger(ai); }
            static Trigger* aspect_of_the_pack(PlayerbotAI* ai) { return new HunterAspectOfThePackTrigger(ai); }
            static Trigger* rapid_fire(PlayerbotAI* ai) { return new RapidFireTrigger(ai); }
            static Trigger* aspect_of_the_hawk(PlayerbotAI* ai) { return new HunterAspectOfTheHawkTrigger(ai); }
            static Trigger* aspect_of_the_wild(PlayerbotAI* ai) { return new HunterAspectOfTheWildTrigger(ai); }
            static Trigger* low_ammo(PlayerbotAI* ai) { return new HunterLowAmmoTrigger(ai); }
            static Trigger* no_ammo(PlayerbotAI* ai) { return new HunterNoAmmoTrigger(ai); }
            static Trigger* has_ammo(PlayerbotAI* ai) { return new HunterHasAmmoTrigger(ai); }
            static Trigger* switch_to_melee(PlayerbotAI* ai) { return new SwitchToMeleeTrigger(ai); }
            static Trigger* switch_to_ranged(PlayerbotAI* ai) { return new SwitchToRangedTrigger(ai); }
            static Trigger* feign_death(PlayerbotAI* ai) { return new FeignDeathTrigger(ai); }
            static Trigger* no_beast(PlayerbotAI* ai) { return new HunterNoPet(ai); }
        };
    };
};

namespace ai
{
    namespace hunter
    {
        using namespace ai;

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["auto shot"] = &AiObjectContextInternal::auto_shot;
                creators["aimed shot"] = &AiObjectContextInternal::aimed_shot;
                creators["chimera shot"] = &AiObjectContextInternal::chimera_shot;
                creators["explosive shot"] = &AiObjectContextInternal::explosive_shot;
                creators["arcane shot"] = &AiObjectContextInternal::arcane_shot;
                creators["tranquilizing shot"] = &AiObjectContextInternal::tranquilizing_shot;
                creators["concussive shot"] = &AiObjectContextInternal::concussive_shot;
                creators["distracting shot"] = &AiObjectContextInternal::distracting_shot;
                creators["multi shot"] = &AiObjectContextInternal::multi_shot;
                creators["volley"] = &AiObjectContextInternal::volley;
                creators["serpent sting"] = &AiObjectContextInternal::serpent_sting;
                creators["serpent sting on attacker"] = &AiObjectContextInternal::serpent_sting_on_attacker;
                creators["viper sting on attacker"] = &AiObjectContextInternal::viper_sting_on_attacker;
                creators["wyvern sting"] = &AiObjectContextInternal::wyvern_sting;
                creators["viper sting"] = &AiObjectContextInternal::viper_sting;
                creators["scorpid sting"] = &AiObjectContextInternal::scorpid_sting;
                creators["hunter's mark"] = &AiObjectContextInternal::hunters_mark;
                creators["mend pet"] = &AiObjectContextInternal::mend_pet;
                creators["revive pet"] = &AiObjectContextInternal::revive_pet;
                creators["call pet"] = &AiObjectContextInternal::call_pet;
                creators["black arrow"] = &AiObjectContextInternal::black_arrow;
                creators["freezing trap"] = &AiObjectContextInternal::freezing_trap;
                creators["rapid fire"] = &AiObjectContextInternal::rapid_fire;
                creators["boost"] = &AiObjectContextInternal::rapid_fire;
                creators["readiness"] = &AiObjectContextInternal::readiness;
                creators["aspect of the monkey"] = &AiObjectContextInternal::aspect_of_the_monkey;
                creators["aspect of the hawk"] = &AiObjectContextInternal::aspect_of_the_hawk;
                creators["aspect of the wild"] = &AiObjectContextInternal::aspect_of_the_wild;
                creators["aspect of the viper"] = &AiObjectContextInternal::aspect_of_the_viper;
                creators["aspect of the pack"] = &AiObjectContextInternal::aspect_of_the_pack;
                creators["aspect of the cheetah"] = &AiObjectContextInternal::aspect_of_the_cheetah;
                creators["remove aspect of the cheetah"] = &AiObjectContextInternal::remove_aspect_of_the_cheetah;
                creators["trueshot aura"] = &AiObjectContextInternal::trueshot_aura;
                creators["feign death"] = &AiObjectContextInternal::feign_death;
                creators["wing clip"] = &AiObjectContextInternal::wing_clip;
                creators["raptor strike"] = &AiObjectContextInternal::raptor_strike;
                creators["feed pet"] = &AiObjectContextInternal::feed_pet;
                creators["bestial wrath"] = &AiObjectContextInternal::bestial_wrath;
                creators["scare beast"] = &AiObjectContextInternal::scare_beast;
                creators["scare beast on cc"] = &AiObjectContextInternal::scare_beast_on_cc;
                creators["remove feign death"] = &AiObjectContextInternal::remove_feign_death;
                creators["frost trap"] = &AiObjectContextInternal::frost_trap;
                creators["explosive trap"] = &AiObjectContextInternal::explosive_trap;
                creators["scatter shot"] = &AiObjectContextInternal::scatter_shot;
                creators["intimidation"] = &AiObjectContextInternal::intimidation;
                creators["deterrence"] = &AiObjectContextInternal::deterrence;
                creators["counterattack"] = &AiObjectContextInternal::counterattack;
                creators["wyvern sting"] = &AiObjectContextInternal::wyvern_sting;
                creators["mongoose bite"] = &AiObjectContextInternal::mongoose_bite;
                creators["black arrow on snare target"] = &AiObjectContextInternal::black_arrow_snare;
                creators["silencing shot"] = &AiObjectContextInternal::silencing_shot;
                creators["silencing shot on enemy healer"] = &AiObjectContextInternal::silencing_shot_healer;
                creators["readiness"] = &AiObjectContextInternal::readiness;
                creators["steady shot"] = &AiObjectContextInternal::steady_shot;
                creators["tame beast"] = &AiObjectContextInternal::tame_beast;
            }

        private:
            static Action* steady_shot(PlayerbotAI* ai) { return new CastSteadyShotAction(ai); }
            static Action* silencing_shot_healer(PlayerbotAI* ai) { return new CastSilencingShotOnHealerAction(ai); }
            static Action* silencing_shot(PlayerbotAI* ai) { return new CastSilencingShotAction(ai); }
            static Action* black_arrow_snare(PlayerbotAI* ai) { return new CastBlackArrowSnareAction(ai); }
            static Action* mongoose_bite(PlayerbotAI* ai) { return new MongooseBiteAction(ai); }
            static Action* wyvern_sting(PlayerbotAI* ai) { return new WyvernStingSnareAction(ai); }
            static Action* counterattack(PlayerbotAI* ai) { return new CastCounterattackAction(ai); }
            static Action* deterrence(PlayerbotAI* ai) { return new DeterrenceAction(ai); }
            static Action* intimidation(PlayerbotAI* ai) { return new IntimidationAction(ai); }
            static Action* scatter_shot(PlayerbotAI* ai) { return new CastScatterShotAction(ai); }
            static Action* explosive_trap(PlayerbotAI* ai) { return new CastExplosiveTrapAction(ai); }
            static Action* frost_trap(PlayerbotAI* ai) { return new CastFrostTrapAction(ai); }
            static Action* scare_beast(PlayerbotAI* ai) { return new CastScareBeastAction(ai); }
            static Action* scare_beast_on_cc(PlayerbotAI* ai) { return new CastScareBeastCcAction(ai); }
            static Action* bestial_wrath(PlayerbotAI* ai) { return new CastBestialWrathAction(ai); }
            static Action* feed_pet(PlayerbotAI* ai) { return new FeedPetAction(ai); }
            static Action* feign_death(PlayerbotAI* ai) { return new CastFeignDeathAction(ai); }
            static Action* trueshot_aura(PlayerbotAI* ai) { return new CastTrueshotAuraAction(ai); }
            static Action* auto_shot(PlayerbotAI* ai) { return new CastAutoShotAction(ai); }
            static Action* aimed_shot(PlayerbotAI* ai) { return new CastAimedShotAction(ai); }
            static Action* chimera_shot(PlayerbotAI* ai) { return new CastChimeraShotAction(ai); }
            static Action* explosive_shot(PlayerbotAI* ai) { return new CastExplosiveShotAction(ai); }
            static Action* tranquilizing_shot(PlayerbotAI* ai) { return new CastTranquilizingShotAction(ai); }
            static Action* arcane_shot(PlayerbotAI* ai) { return new CastArcaneShotAction(ai); }
            static Action* concussive_shot(PlayerbotAI* ai) { return new CastConcussiveShotAction(ai); }
            static Action* distracting_shot(PlayerbotAI* ai) { return new CastDistractingShotAction(ai); }
            static Action* multi_shot(PlayerbotAI* ai) { return new CastMultiShotAction(ai); }
            static Action* volley(PlayerbotAI* ai) { return new CastVolleyAction(ai); }
            static Action* serpent_sting(PlayerbotAI* ai) { return new CastSerpentStingAction(ai); }
            static Action* serpent_sting_on_attacker(PlayerbotAI* ai) { return new CastSerpentStingOnAttackerAction(ai); }
            static Action* viper_sting_on_attacker(PlayerbotAI* ai) { return new CastViperStingOnAttackerAction(ai); }
            static Action* viper_sting(PlayerbotAI* ai) { return new CastViperStingAction(ai); }
            static Action* scorpid_sting(PlayerbotAI* ai) { return new CastScorpidStingAction(ai); }
            static Action* hunters_mark(PlayerbotAI* ai) { return new CastHuntersMarkAction(ai); }
            static Action* mend_pet(PlayerbotAI* ai) { return new CastMendPetAction(ai); }
            static Action* revive_pet(PlayerbotAI* ai) { return new CastRevivePetAction(ai); }
            static Action* call_pet(PlayerbotAI* ai) { return new CastCallPetAction(ai); }
            static Action* black_arrow(PlayerbotAI* ai) { return new CastBlackArrow(ai); }
            static Action* freezing_trap(PlayerbotAI* ai) { return new CastFreezingTrapAction(ai); }
            static Action* rapid_fire(PlayerbotAI* ai) { return new CastRapidFireAction(ai); }
            static Action* readiness(PlayerbotAI* ai) { return new CastReadinessAction(ai); }
            static Action* aspect_of_the_monkey(PlayerbotAI* ai) { return new CastAspectOfTheMonkeyAction(ai); }
            static Action* aspect_of_the_hawk(PlayerbotAI* ai) { return new CastAspectOfTheHawkAction(ai); }
            static Action* aspect_of_the_wild(PlayerbotAI* ai) { return new CastAspectOfTheWildAction(ai); }
            static Action* aspect_of_the_viper(PlayerbotAI* ai) { return new CastAspectOfTheViperAction(ai); }
            static Action* aspect_of_the_pack(PlayerbotAI* ai) { return new CastAspectOfThePackAction(ai); }
            static Action* aspect_of_the_cheetah(PlayerbotAI* ai) { return new CastAspectOfTheCheetahAction(ai); }
            static Action* remove_aspect_of_the_cheetah(PlayerbotAI* ai) { return new RemoveBuffAction(ai, "aspect of the cheetah"); }
            static Action* wing_clip(PlayerbotAI* ai) { return new CastWingClipAction(ai); }
            static Action* raptor_strike(PlayerbotAI* ai) { return new CastRaptorStrikeAction(ai); }
            static Action* remove_feign_death(PlayerbotAI* ai) { return new RemoveFeignDeathAction(ai); }
            static Action* tame_beast(PlayerbotAI* ai) { return new TameBeastAction(ai); }
        };
    };
};

HunterAiObjectContext::HunterAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::hunter::StrategyFactoryInternal());
    strategyContexts.Add(new ai::hunter::BuffStrategyFactoryInternal());
    actionContexts.Add(new ai::hunter::AiObjectContextInternal());
    triggerContexts.Add(new ai::hunter::TriggerFactoryInternal());
}
