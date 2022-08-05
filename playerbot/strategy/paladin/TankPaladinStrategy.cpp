#include "botpch.h"
#include "../../playerbot.h"
#include "PaladinMultipliers.h"
#include "TankPaladinStrategy.h"

using namespace ai;

class TankPaladinStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    TankPaladinStrategyActionNodeFactory()
    {
        creators["seal of vengeance"] = &seal_of_vengeance;
        creators["hand of reckoning"] = &hand_of_reckoning;
        creators["judgement"] = &judgement;
    }

private:
    static ActionNode* seal_of_vengeance(PlayerbotAI* ai)
    {
        return new ActionNode("seal of vengeance",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("seal of righteousness"), NULL),
            /*C*/ NULL);
    }
    ACTION_NODE_A(hand_of_reckoning, "hand of reckoning", "righteous defense");
    ACTION_NODE_A(judgement, "judgement", "exorcism");
};

TankPaladinStrategy::TankPaladinStrategy(PlayerbotAI* ai) : GenericPaladinStrategy(ai)
{
    actionNodeFactories.Add(new TankPaladinStrategyActionNodeFactory());
}

NextAction** TankPaladinStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("melee", ACTION_NORMAL), NULL);
}

void TankPaladinStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    GenericPaladinStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "seal",
        NextAction::array(0, new NextAction("seal of vengeance", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "paladin aura",
        NextAction::array(0, new NextAction("retribution aura", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("seal of wisdom", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "judgement of light",
        NextAction::array(0, new NextAction("judgement of light", ACTION_HIGH + 10), NULL)));

    triggers.push_back(new TriggerNode(
        "medium mana",
        NextAction::array(0, new NextAction("judgement of wisdom", ACTION_HIGH + 10), NULL)));

    triggers.push_back(new TriggerNode(
        "judgement",
        NextAction::array(0, new NextAction("judgement", ACTION_HIGH + 10), NULL)));

    triggers.push_back(new TriggerNode(
        "judgement",
        NextAction::array(0, new NextAction("exorcism", ACTION_HIGH + 9), NULL)));

    triggers.push_back(new TriggerNode(
        "light aoe",
        NextAction::array(0, new NextAction("hammer of the righteous", ACTION_HIGH + 8), new NextAction("avenger's shield", ACTION_HIGH + 7), NULL)));

    triggers.push_back(new TriggerNode(
        "avenger's shield",
        NextAction::array(0, new NextAction("avenger's shield", ACTION_HIGH + 7), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy is close",
        NextAction::array(0, new NextAction("consecration", ACTION_INTERRUPT + 10), NULL)));

    triggers.push_back(new TriggerNode(
        "hammer of justice on enemy",
        NextAction::array(0, new NextAction("hammer of justice", ACTION_INTERRUPT + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "lose aggro",
        NextAction::array(0, new NextAction("hand of reckoning", ACTION_HIGH + 7), NULL)));

    triggers.push_back(new TriggerNode(
        "lose aggro",
        NextAction::array(0, new NextAction("exorcism", ACTION_HIGH + 8), NULL)));

	triggers.push_back(new TriggerNode(
		"holy shield",
		NextAction::array(0, new NextAction("holy shield", ACTION_HIGH + 7), NULL)));

    triggers.push_back(new TriggerNode(
        "blessing",
        NextAction::array(0, new NextAction("blessing of sanctuary", ACTION_HIGH + 9), NULL)));

    triggers.push_back(new TriggerNode(
        "target critical health",
        NextAction::array(0, new NextAction("hammer of wrath", ACTION_CRITICAL_HEAL), NULL)));

    triggers.push_back(new TriggerNode(
        "light aoe",
        NextAction::array(0, new NextAction("goblin sapper", ACTION_HIGH + 8), NULL)));

    triggers.push_back(new TriggerNode(
        "light aoe",
        NextAction::array(0, new NextAction("oil of immolation", ACTION_HIGH + 9), NULL)));

    triggers.push_back(new TriggerNode(
        "light aoe",
        NextAction::array(0, new NextAction("avenging wrath", ACTION_HIGH + 9), NULL)));
}
