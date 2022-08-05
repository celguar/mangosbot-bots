#include "botpch.h"
#include "../../playerbot.h"
#include "PaladinMultipliers.h"
#include "PaladinBuffStrategies.h"

using namespace ai;

void PaladinBuffWisdomStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "blessing on party",
        NextAction::array(0, new NextAction("blessing of wisdom on party", 11.0f), NULL)));
}

void PaladinBuffSpeedStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "crusader aura",
        NextAction::array(0, new NextAction("crusader aura", 40.0f), NULL)));
}

void PaladinBuffMightStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "blessing on party",
        NextAction::array(0, new NextAction("blessing of might on party", 11.0f), NULL)));
}

void PaladinShadowResistanceStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
	triggers.push_back(new TriggerNode(
		"paladin aura",
		NextAction::array(0, new NextAction("shadow resistance aura", 12.0f), NULL)));
}

void PaladinFrostResistanceStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
	triggers.push_back(new TriggerNode(
		"paladin aura",
		NextAction::array(0, new NextAction("frost resistance aura", 12.0f), NULL)));
}

void PaladinFireResistanceStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
	triggers.push_back(new TriggerNode(
		"paladin aura",
		NextAction::array(0, new NextAction("fire resistance aura", 12.0f), NULL)));
}

void PaladinBuffArmorStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
	triggers.push_back(new TriggerNode(
		"paladin aura",
		NextAction::array(0, new NextAction("devotion aura", 12.0f), NULL)));
}

void PaladinBuffConcentrationStrategy::InitTriggers(std::vector<TriggerNode*> & triggers)
{
    triggers.push_back(new TriggerNode(
        "paladin aura",
        NextAction::array(0, new NextAction("concentration aura", 12.0f), NULL)));
}

void PaladinBuffSanctityStrategy::InitTriggers(std::vector<TriggerNode*> & triggers)
{
    triggers.push_back(new TriggerNode(
        "paladin aura",
        NextAction::array(0, new NextAction("sanctity aura", 12.0f), NULL)));
}

void PaladinBuffAoeStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "paladin aura",
        NextAction::array(0, new NextAction("retribution aura", 12.0f), NULL)));
}

void PaladinBuffThreatStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "righteous fury",
        NextAction::array(0, new NextAction("righteous fury", ACTION_HIGH + 8), NULL)));
}

void PaladinBuffKingsStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "blessing on party",
        NextAction::array(0, new NextAction("blessing of kings on party", 11.0f), NULL)));
}
