#include "botpch.h"
#include "../../playerbot.h"
#include "GenericHunterStrategy.h"
#include "HunterAiObjectContext.h"

using namespace ai;

class GenericHunterStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericHunterStrategyActionNodeFactory()
    {
        creators["rapid fire"] = &rapid_fire;
        creators["boost"] = &rapid_fire;
        creators["aspect of the pack"] = &aspect_of_the_pack;
        creators["wing clip"] = &wing_clip;
        creators["raptor strike"] = &raptor_strike;
        creators["freezing trap"] = &freezing_trap;
    }
private:
    static ActionNode* rapid_fire(PlayerbotAI* ai)
    {
        return new ActionNode ("rapid fire",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("readiness"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* aspect_of_the_pack(PlayerbotAI* ai)
    {
        return new ActionNode ("aspect of the pack",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("aspect of the cheetah"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* wing_clip(PlayerbotAI* ai)
    {
        return new ActionNode("wing clip",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("raptor strike"), NULL),
            /*C*/ NextAction::array(0, new NextAction("flee"), NULL));
    }
    static ActionNode* raptor_strike(PlayerbotAI* ai)
    {
        return new ActionNode("raptor strike",
            /*P*/ NextAction::array(0, new NextAction("melee"), NULL),
            /*A*/ NULL,
            /*C*/ NULL);
    }
    ACTION_NODE_A(freezing_trap, "freezing trap", "remove feign death");
};

GenericHunterStrategy::GenericHunterStrategy(PlayerbotAI* ai) : CombatStrategy(ai)
{
    actionNodeFactories.Add(new GenericHunterStrategyActionNodeFactory());
}

void GenericHunterStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy is close",
        NextAction::array(0, new NextAction("wing clip", ACTION_HIGH), NULL)));

    /*triggers.push_back(new TriggerNode(
        "enemy is close",
        NextAction::array(0, new NextAction("freezing trap", ACTION_HIGH + 5), NULL)));*/

    triggers.push_back(new TriggerNode(
        "medium threat",
        NextAction::array(0, new NextAction("feign death", ACTION_INTERRUPT + 1), new NextAction("shadowmeld", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "feign death",
        NextAction::array(0, new NextAction("remove feign death", ACTION_INTERRUPT + 1), new NextAction("freezing trap", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "hunters pet low health",
        NextAction::array(0, new NextAction("mend pet", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "switch to melee",
        NextAction::array(0, new NextAction("switch to melee", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "switch to ranged",
        NextAction::array(0, new NextAction("switch to ranged", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "no ammo",
        NextAction::array(0, new NextAction("switch to melee", ACTION_HIGH + 1), new NextAction("say::no ammo", ACTION_HIGH), NULL)));
}

NextAction** HunterBoostStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("bestial wrath", 15.0f), NULL);
}

void HunterBoostStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "rapid fire",
        NextAction::array(0, new NextAction("rapid fire", 16.0f), NULL)));
}

void HunterCcStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "scare beast",
        NextAction::array(0, new NextAction("scare beast on cc", ACTION_HIGH + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "freezing trap",
        NextAction::array(0, new NextAction("freezing trap on cc", ACTION_HIGH + 3), NULL)));
}
