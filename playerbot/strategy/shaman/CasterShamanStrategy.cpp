#include "botpch.h"
#include "../../playerbot.h"
#include "ShamanMultipliers.h"
#include "CasterShamanStrategy.h"

using namespace ai;

class CasterShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    CasterShamanStrategyActionNodeFactory()
    {
        creators["fire totem"] = &fire_totem;
        creators["fire totem aoe"] = &fire_totem_aoe;
        creators["earth totem"] = &earth_totem;
        creators["air totem"] = &air_totem;
        creators["water totem"] = &water_totem;
    }
private:
    static ActionNode* fire_totem(PlayerbotAI* ai)
    {
        return new ActionNode("totem of wrath",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("flametongue totem"), NULL),
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
        return new ActionNode("mana spring totem",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("healing stream totem"), NULL),
            /*C*/ NULL);
    }
};

CasterShamanStrategy::CasterShamanStrategy(PlayerbotAI* ai) : GenericShamanStrategy(ai)
{
    actionNodeFactories.Add(new CasterShamanStrategyActionNodeFactory());
}

NextAction** CasterShamanStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("lightning bolt", 10.0f), NULL);
}

void CasterShamanStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    GenericShamanStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy out of spell",
        NextAction::array(0, new NextAction("reach spell", ACTION_NORMAL + 9), NULL)));

    triggers.push_back(new TriggerNode(
        "shock",
        NextAction::array(0, new NextAction("earth shock", 20.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "frost shock snare",
        NextAction::array(0, new NextAction("frost shock", 21.0f), NULL)));
}

void CasterAoeShamanStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "light aoe",
        NextAction::array(0, new NextAction("chain lightning", 25.0f), NULL)));
}
