#include "botpch.h"
#include "../../playerbot.h"
#include "RogueTriggers.h"
#include "RogueMultipliers.h"
#include "GenericRogueNonCombatStrategy.h"
#include "RogueActions.h"

using namespace ai;

void GenericRogueNonCombatStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
	//TODO Possible crash on CMSG_USE_ITEM ByteBuffer error
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("sprint", ACTION_INTERRUPT + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy flagcarrier near",
        NextAction::array(0, new NextAction("sprint", ACTION_INTERRUPT + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "unstealth",
        NextAction::array(0, new NextAction("unstealth", 1.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("apply poison", 1.0f), NULL)));
}
