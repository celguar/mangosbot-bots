#include "botpch.h"
#include "../../playerbot.h"
#include "ShamanMultipliers.h"
#include "MeleeShamanStrategy.h"

using namespace ai;

class MeleeShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    MeleeShamanStrategyActionNodeFactory()
    {
        creators["stormstrike"] = &stormstrike;
        creators["lava lash"] = &lava_lash;
        creators["fire totem"] = &fire_totem;
        creators["fire totem aoe"] = &fire_totem_aoe;
        creators["earth totem"] = &earth_totem;
        creators["air totem"] = &air_totem;
        creators["water totem"] = &water_totem;
    }
private:
    static ActionNode* stormstrike(PlayerbotAI* ai)
    {
        return new ActionNode ("stormstrike",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("lava lash"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* lava_lash(PlayerbotAI* ai)
    {
        return new ActionNode ("lava lash",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("melee"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* fire_totem(PlayerbotAI* ai)
    {
        return new ActionNode("flametongue totem",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("searing totem"), NULL),
            /*C*/ NextAction::array(0, new NextAction("reach melee", 22.0f), NULL));
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
        return new ActionNode("windfury totem",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("wrath of air totem"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* water_totem(PlayerbotAI* ai)
    {
        return new ActionNode("mana spring totem",
            /*P*/ NULL,
            /*A*/ NULL,
            /*C*/ NULL);
    }
};

MeleeShamanStrategy::MeleeShamanStrategy(PlayerbotAI* ai) : GenericShamanStrategy(ai)
{
    actionNodeFactories.Add(new MeleeShamanStrategyActionNodeFactory());
}

NextAction** MeleeShamanStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("stormstrike", ACTION_HIGH), NULL);
}

void MeleeShamanStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    GenericShamanStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array(0, new NextAction("windfury weapon", 22.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "shock",
        NextAction::array(0, new NextAction("earth shock", 20.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "frost shock snare",
        NextAction::array(0, new NextAction("frost shock", 21.0f), NULL)));
}

void MeleeAoeShamanStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "medium aoe",
        NextAction::array(0, new NextAction("fire nova", 25.0f), NULL)));
}
