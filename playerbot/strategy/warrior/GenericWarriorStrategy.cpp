#include "botpch.h"
#include "../../playerbot.h"
#include "GenericWarriorStrategy.h"
#include "WarriorAiObjectContext.h"

using namespace ai;

GenericWarriorStrategy::GenericWarriorStrategy(PlayerbotAI* ai) : CombatStrategy(ai)
{
    //actionNodeFactories.Add(new WarriorStanceRequirementActionNodeFactory());
}

void GenericWarriorStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    CombatStrategy::InitTriggers(triggers);

    /*triggers.push_back(new TriggerNode(
        "bloodrage",
        NextAction::array(0, new NextAction("bloodrage", ACTION_HIGH + 1), NULL)));*/

    /*triggers.push_back(new TriggerNode(
        "shield bash",
        NextAction::array(0, new NextAction("shield bash", ACTION_INTERRUPT + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "shield bash on enemy healer",
        NextAction::array(0, new NextAction("shield bash on enemy healer", ACTION_INTERRUPT + 3), NULL)));*/

	/*triggers.push_back(new TriggerNode(
		"critical health",
		NextAction::array(0, new NextAction("intimidating shout", ACTION_EMERGENCY), NULL)));*/
}

class WarrirorAoeStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    WarrirorAoeStrategyActionNodeFactory()
    {
        creators["whirlwind"] = &whirlwind;
    }
private:
    ACTION_NODE_A(whirlwind, "whirlwind", "cleave");
};

WarrirorAoeStrategy::WarrirorAoeStrategy(PlayerbotAI* ai) : CombatStrategy(ai)
{
    actionNodeFactories.Add(new WarrirorAoeStrategyActionNodeFactory());
}

void WarrirorAoeStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "thunder clap on snare target",
        NextAction::array(0, new NextAction("thunder clap on snare target", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "thunder clap",
        NextAction::array(0, new NextAction("thunder clap", ACTION_NORMAL + 10), NULL)));

    triggers.push_back(new TriggerNode(
        "light aoe",
        NextAction::array(0, new NextAction("demoralizing shout", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "shockwave on snare target",
        NextAction::array(0, new NextAction("shockwave on snare target", ACTION_NORMAL + 5), NULL)));

    triggers.push_back(new TriggerNode(
        "shockwave",
        NextAction::array(0, new NextAction("shockwave", ACTION_NORMAL + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "light aoe",
        NextAction::array(0, new NextAction("sweeping strikes", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "light aoe",
        NextAction::array(0, new NextAction("bladestorm", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "light aoe",
        NextAction::array(0, new NextAction("whirlwind", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "high rage available",
        NextAction::array(0, new NextAction("whirlwind", ACTION_HIGH + 9), NULL)));
}
