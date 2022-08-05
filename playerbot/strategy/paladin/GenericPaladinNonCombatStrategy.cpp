#include "botpch.h"
#include "../../playerbot.h"
#include "PaladinMultipliers.h"
#include "GenericPaladinNonCombatStrategy.h"
#include "GenericPaladinStrategyActionNodeFactory.h"

using namespace ai;

GenericPaladinNonCombatStrategy::GenericPaladinNonCombatStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai)
{
    actionNodeFactories.Add(new GenericPaladinStrategyActionNodeFactory());
}

void GenericPaladinNonCombatStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

	triggers.push_back(new TriggerNode(
		"party member dead",
		NextAction::array(0, new NextAction("redemption", 30.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "almost full health",
        NextAction::array(0, new NextAction("flash of light", 23.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "party member almost full health",
        NextAction::array(0, new NextAction("flash of light on party", 24.0f), NULL)));

	 triggers.push_back(new TriggerNode(
        "medium health",
        NextAction::array(0, new NextAction("flash of light", 25.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array(0, new NextAction("flash of light on party", 26.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("holy light", 50.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("holy light on party", 40.0f), NULL)));

    triggers.push_back(new TriggerNode(
       "often",
       NextAction::array(0, new NextAction("apply stone", 1.0f), NULL)));
}
