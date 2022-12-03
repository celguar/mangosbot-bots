#include "botpch.h"
#include "../../playerbot.h"
#include "ShamanMultipliers.h"
#include "HealShamanStrategy.h"

using namespace ai;

class HealShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    HealShamanStrategyActionNodeFactory()
    {
        creators["earthliving weapon"] = &earthliving_weapon;
        creators["mana tide totem"] = &mana_tide_totem;
        creators["fire totem"] = &fire_totem;
        creators["fire totem aoe"] = &fire_totem_aoe;
        creators["earth totem"] = &earth_totem;
        creators["air totem"] = &air_totem;
        creators["water totem"] = &water_totem;
    }
private:
    static ActionNode* earthliving_weapon(PlayerbotAI* ai)
    {
        return new ActionNode ("earthliving weapon",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("flametongue weapon"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* mana_tide_totem(PlayerbotAI* ai)
    {
        return new ActionNode ("mana tide totem",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("mana potion"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* fire_totem(PlayerbotAI* ai)
    {
        return new ActionNode("flametongue totem",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("searing totem"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* fire_totem_aoe(PlayerbotAI* ai)
    {
        return new ActionNode("magma totem",
            /*P*/ NULL,
            /*A*/ NULL,
            /*C*/ NextAction::array(0, new NextAction("fire nova"), NULL));
    }
    static ActionNode* earth_totem(PlayerbotAI* ai)
    {
        return new ActionNode("strength of earth totem",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("stoneskin totem"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* air_totem(PlayerbotAI* ai)
    {
        return new ActionNode("wrath of air totem",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("windfury totem"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* water_totem(PlayerbotAI* ai)
    {
        return new ActionNode("healing stream totem",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("mana spring totem"), NULL),
            /*C*/ NULL);
    }
};

HealShamanStrategy::HealShamanStrategy(PlayerbotAI* ai) : GenericShamanStrategy(ai)
{
    actionNodeFactories.Add(new HealShamanStrategyActionNodeFactory());
}

void HealShamanStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    GenericShamanStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array(0, new NextAction("earthliving weapon", 22.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("mana tide totem", ACTION_EMERGENCY + 5), NULL)));

    triggers.push_back(new TriggerNode(
        "party member to heal out of spell range",
        NextAction::array(0, new NextAction("reach party member to heal", ACTION_CRITICAL_HEAL + 1), NULL)));
}
