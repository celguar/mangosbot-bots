#include "botpch.h"
#include "../../playerbot.h"
#include "MageMultipliers.h"
#include "FrostMageStrategy.h"

using namespace ai;


FrostMageStrategy::FrostMageStrategy(PlayerbotAI* ai) : GenericMageStrategy(ai)
{
}

NextAction** FrostMageStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("frostbolt", 7.0f), NULL);
}

void FrostMageStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    GenericMageStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "icy veins",
        NextAction::array(0, new NextAction("icy veins", 50.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "has aggro",
        NextAction::array(0, new NextAction("ice barrier", 9.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "ice lance",
        NextAction::array(0, new NextAction("ice lance", 61.0f), NULL)));

    /*triggers.push_back(new TriggerNode(
        "has aggro",
        NextAction::array(0, new NextAction("fire blast", 6.0f), NULL)));*/

    triggers.push_back(new TriggerNode(
        "summon water elemental",
        NextAction::array(0, new NextAction("summon water elemental", 10.0f), NULL)));
}

void FrostMageAoeStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
	triggers.push_back(new TriggerNode(
		"medium aoe",
		NextAction::array(0, new NextAction("blizzard", 40.0f), NULL)));
}
