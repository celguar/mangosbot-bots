#include "botpch.h"
#include "../../playerbot.h"
#include "ShamanMultipliers.h"
#include "TotemsShamanStrategy.h"

using namespace ai;

void TotemsShamanStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    GenericShamanStrategy::InitCombatTriggers(triggers);

    //sLog.outError("TotemsShamanStrategy - setting triggers fire");
    triggers.push_back(new TriggerNode(
        "fire totem",
        NextAction::array(0, new NextAction("fire totem", 25.0f), NULL)));;

    //sLog.outError("TotemsShamanStrategy - setting triggers aoe");
    triggers.push_back(new TriggerNode(
        "fire totem aoe",
        NextAction::array(0, new NextAction("fire totem aoe", 26.0f), NULL)));

    //sLog.outError("TotemsShamanStrategy - setting triggers earth");
    triggers.push_back(new TriggerNode(
        "earth totem",
        NextAction::array(0, new NextAction("earth totem", 27.0f), NULL)));

    //sLog.outError("TotemsShamanStrategy - setting triggers water");
    triggers.push_back(new TriggerNode(
        "water totem",
        NextAction::array(0, new NextAction("water totem", 28.0f), NULL)));

    //sLog.outError("TotemsShamanStrategy - setting triggers air");
    triggers.push_back(new TriggerNode(
        "air totem",
        NextAction::array(0, new NextAction("air totem", 29.0f), NULL)));
}

void ShamanTotemBarElementsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitCombatTriggers(triggers);
    triggers.push_back(new TriggerNode(
        "call of the elements",
        NextAction::array(0, new NextAction("call of the elements", 30.0f), NULL)));
}

void ShamanTotemBarAncestorsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitCombatTriggers(triggers);;
    triggers.push_back(new TriggerNode(
        "call of the ancestors",
        NextAction::array(0, new NextAction("call of the ancestors", 30.0f), NULL)));
}

void ShamanTotemBarSpiritsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitCombatTriggers(triggers);
    triggers.push_back(new TriggerNode(
        "call of the ancestors",
        NextAction::array(0, new NextAction("call of the spirits", 30.0f), NULL)));
}