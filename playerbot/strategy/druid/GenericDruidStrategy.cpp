#include "botpch.h"
#include "../../playerbot.h"
#include "GenericDruidStrategy.h"
#include "DruidAiObjectContext.h"

using namespace ai;

class GenericDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericDruidStrategyActionNodeFactory()
    {
        creators["melee"] = &melee;
        creators["caster form"] = &caster_form;
        creators["cure poison"] = &cure_poison;
        creators["cure poison on party"] = &cure_poison_on_party;
        creators["abolish poison"] = &abolish_poison;
        creators["abolish poison on party"] = &abolish_poison_on_party;
        creators["rebirth"] = &rebirth;
        creators["entangling roots on cc"] = &entangling_roots_on_cc;
        creators["innervate"] = &innervate;
    }

private:
    static ActionNode* melee(PlayerbotAI* ai)
    {
        return new ActionNode ("melee",
            /*P*/ NULL,
            /*A*/ NULL,
            /*C*/ NULL);
    }
    static ActionNode* caster_form(PlayerbotAI* ai)
    {
        return new ActionNode ("caster form",
            /*P*/ NULL,
            /*A*/ NULL,
            /*C*/ NULL);
    }
    static ActionNode* cure_poison(PlayerbotAI* ai)
    {
        return new ActionNode ("cure poison",
            /*P*/ NULL,
            /*A*/ NULL,
            /*C*/ NULL);
    }
    static ActionNode* cure_poison_on_party(PlayerbotAI* ai)
    {
        return new ActionNode ("cure poison on party",
            /*P*/ NULL,
            /*A*/ NULL,
            /*C*/ NULL);
    }
    static ActionNode* abolish_poison(PlayerbotAI* ai)
    {
        return new ActionNode ("abolish poison",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("cure poison"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* abolish_poison_on_party(PlayerbotAI* ai)
    {
        return new ActionNode ("abolish poison on party",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("cure poison on party"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* rebirth(PlayerbotAI* ai)
    {
        return new ActionNode ("rebirth",
            /*P*/ NULL,
            /*A*/ NULL,
            /*C*/ NULL);
    }
    static ActionNode* entangling_roots_on_cc(PlayerbotAI* ai)
    {
        return new ActionNode ("entangling roots on cc",
            /*P*/ NextAction::array(0, new NextAction("caster form"), NULL),
            /*A*/ NULL,
            /*C*/ NULL);
    }
    static ActionNode* innervate(PlayerbotAI* ai)
    {
        return new ActionNode ("innervate",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("mana potion"), NULL),
            /*C*/ NULL);
    }
};

GenericDruidStrategy::GenericDruidStrategy(PlayerbotAI* ai) : CombatStrategy(ai)
{
    actionNodeFactories.Add(new GenericDruidStrategyActionNodeFactory());
}

void GenericDruidStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    CombatStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("regrowth", ACTION_MEDIUM_HEAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("regrowth on party", ACTION_MEDIUM_HEAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("regrowth", ACTION_CRITICAL_HEAL + 2), new NextAction("healing touch", ACTION_CRITICAL_HEAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0,  new NextAction("regrowth on party", ACTION_CRITICAL_HEAL + 2), new NextAction("healing touch on party", ACTION_CRITICAL_HEAL + 1), NULL)));

	triggers.push_back(new TriggerNode(
		"party member dead",
		NextAction::array(0, new NextAction("rebirth", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("innervate", ACTION_HIGH + 5), NULL)));

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("travel form", ACTION_EMERGENCY + 2), NULL)));
}

void DruidCureStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "cure poison",
        NextAction::array(0, new NextAction("abolish poison", ACTION_DISPEL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "party member cure poison",
        NextAction::array(0, new NextAction("abolish poison on party", ACTION_DISPEL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "remove curse",
        NextAction::array(0, new NextAction("remove curse", ACTION_MEDIUM_HEAL + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "party member remove curse",
        NextAction::array(0, new NextAction("remove curse on party", ACTION_MEDIUM_HEAL + 3), NULL)));
}

void DruidCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    InitCombatTriggers(triggers);
}

void DruidBoostStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "nature's swiftness",
        NextAction::array(0, new NextAction("nature's swiftness", ACTION_HIGH + 9), NULL)));
}

void DruidCcStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "entangling roots",
        NextAction::array(0, new NextAction("entangling roots on cc", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "entangling roots kite",
        NextAction::array(0, new NextAction("entangling roots", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "hibernate",
        NextAction::array(0, new NextAction("hibernate on cc", ACTION_HIGH + 3), NULL)));
}
