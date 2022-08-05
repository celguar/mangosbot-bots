#include "botpch.h"
#include "../../playerbot.h"
#include "PaladinMultipliers.h"
#include "DpsPaladinStrategy.h"

using namespace ai;

class DpsPaladinStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DpsPaladinStrategyActionNodeFactory()
    {
        creators["sanctity aura"] = &sanctity_aura;
        creators["retribution aura"] = &retribution_aura;
        creators["seal of vengeance"] = &seal_of_vengeance;
        creators["seal of command"] = &seal_of_command;
        creators["blessing of might"] = &blessing_of_might;
        creators["crusader strike"] = &crusader_strike;
        creators["repentance"] = &repentance;
        creators["repentance on enemy healer"] = &repentance_on_enemy_healer;
        creators["repentance on snare target"] = &repentance_on_snare_target;
        creators["repentance of shield"] = &repentance_or_shield;
        creators["judgement"] = &judgement;
    }

private:
    static ActionNode* seal_of_vengeance(PlayerbotAI* ai)
    {
        return new ActionNode ("seal of vengeance",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("seal of command"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* seal_of_command(PlayerbotAI* ai)
    {
        return new ActionNode ("seal of command",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("seal of righteousness"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* blessing_of_might(PlayerbotAI* ai)
    {
        return new ActionNode ("blessing of might",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("blessing of kings"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* crusader_strike(PlayerbotAI* ai)
    {
        return new ActionNode ("crusader strike",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("melee"), NULL),
            /*C*/ NULL);
    }
    ACTION_NODE_A(repentance, "repentance", "hammer of justice");
    ACTION_NODE_A(repentance_on_enemy_healer, "repentance on enemy healer", "hammer of justice on enemy healer");
    ACTION_NODE_A(repentance_on_snare_target, "repentance on snare target", "hammer of justice on snare target");
    ACTION_NODE_A(sanctity_aura, "sanctity aura", "retribution aura");
    ACTION_NODE_A(retribution_aura, "retribution aura", "devotion aura");
    ACTION_NODE_A(repentance_or_shield, "repentance", "divine shield");
    ACTION_NODE_A(judgement, "judgement", "exorcism");
};

DpsPaladinStrategy::DpsPaladinStrategy(PlayerbotAI* ai) : GenericPaladinStrategy(ai)
{
    actionNodeFactories.Add(new DpsPaladinStrategyActionNodeFactory());
}

NextAction** DpsPaladinStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("crusader strike", ACTION_NORMAL + 1), NULL);
}

void DpsPaladinStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    GenericPaladinStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "seal",
        NextAction::array(0, new NextAction("seal of command", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("seal of wisdom", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "paladin aura",
        NextAction::array(0, new NextAction("sanctity aura", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("repentance or shield", ACTION_CRITICAL_HEAL + 3), new NextAction("holy light", ACTION_CRITICAL_HEAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "judgement of wisdom",
        NextAction::array(0, new NextAction("judgement of wisdom", ACTION_HIGH + 10), NULL)));

    triggers.push_back(new TriggerNode(
        "judgement",
        NextAction::array(0, new NextAction("judgement", ACTION_HIGH + 10), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy is close",
        NextAction::array(0, new NextAction("consecration", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "repentance on enemy healer",
        NextAction::array(0, new NextAction("repentance on enemy healer", ACTION_INTERRUPT + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "repentance on snare target",
        NextAction::array(0, new NextAction("repentance on snare target", ACTION_INTERRUPT + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "repentance interrupt",
        NextAction::array(0, new NextAction("repentance", ACTION_INTERRUPT + 2), NULL)));

	triggers.push_back(new TriggerNode(
		"medium aoe",
		NextAction::array(0, new NextAction("divine storm", ACTION_HIGH + 1), new NextAction("consecration", ACTION_HIGH + 1), NULL)));

	triggers.push_back(new TriggerNode(
		"art of war",
		NextAction::array(0, new NextAction("exorcism", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "target critical health",
        NextAction::array(0, new NextAction("hammer of wrath", ACTION_INTERRUPT + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "hammer of justice on enemy",
        NextAction::array(0, new NextAction("hammer of justice", ACTION_INTERRUPT + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "blessing",
        NextAction::array(0, new NextAction("blessing of might", ACTION_HIGH + 9), NULL)));

    triggers.push_back(new TriggerNode(
        "light aoe",
        NextAction::array(0, new NextAction("avenging wrath", ACTION_HIGH + 9), NULL)));
}
