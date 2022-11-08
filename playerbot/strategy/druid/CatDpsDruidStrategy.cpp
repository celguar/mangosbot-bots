#include "botpch.h"
#include "../../playerbot.h"
#include "DruidMultipliers.h"
#include "CatDpsDruidStrategy.h"

using namespace ai;

class CatDpsDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    CatDpsDruidStrategyActionNodeFactory()
    {
        creators["faerie fire (feral)"] = &faerie_fire_feral;
        creators["melee"] = &melee;
        creators["feral charge - cat"] = &feral_charge_cat;
        creators["cat form"] = &cat_form;
        creators["claw"] = &claw;
        creators["mangle (cat)"] = &mangle_cat;
        creators["rake"] = &rake;
        creators["ferocious bite"] = &ferocious_bite;
        creators["rip"] = &rip;
        creators["pounce"] = &pounce;
        creators["ravage"] = &ravage;
    }
private:
    static ActionNode* faerie_fire_feral(PlayerbotAI* ai)
    {
        return new ActionNode ("faerie fire (feral)",
            /*P*/ NULL,
            /*A*/ NULL,
            /*C*/ NULL);
    }
    static ActionNode* melee(PlayerbotAI* ai)
    {
        return new ActionNode ("melee",
            /*P*/ NextAction::array(0, new NextAction("feral charge - cat"), NULL),
            /*A*/ NULL,
            /*C*/ NULL);
    }
    static ActionNode* feral_charge_cat(PlayerbotAI* ai)
    {
        return new ActionNode ("feral charge - cat",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("reach melee"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* cat_form(PlayerbotAI* ai)
    {
        return new ActionNode ("cat form",
            /*P*/ NULL,
            /*A*/ NULL,
            /*C*/ NULL);
    }
    static ActionNode* claw(PlayerbotAI* ai)
    {
        return new ActionNode ("claw",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("melee"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* mangle_cat(PlayerbotAI* ai)
    {
        return new ActionNode ("mangle (cat)",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("claw"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* rake(PlayerbotAI* ai)
    {
        return new ActionNode ("rake",
            /*P*/ NULL,
            /*A*/ NULL,
            /*C*/ NULL);
    }
    static ActionNode* ferocious_bite(PlayerbotAI* ai)
    {
        return new ActionNode ("ferocious bite",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("rip"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* rip(PlayerbotAI* ai)
    {
        return new ActionNode ("rip",
            /*P*/ NULL,
            /*A*/ NULL,
            /*C*/ NULL);
    }
    static ActionNode* pounce(PlayerbotAI* ai)
    {
        return new ActionNode("pounce",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("ravage"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* ravage(PlayerbotAI* ai)
    {
        return new ActionNode("ravage",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("shred"), NULL),
            /*C*/ NULL);
    }
};

CatDpsDruidStrategy::CatDpsDruidStrategy(PlayerbotAI* ai) : FeralDruidStrategy(ai)
{
    actionNodeFactories.Add(new CatDpsDruidStrategyActionNodeFactory());
}

NextAction** CatDpsDruidStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("mangle (cat)", ACTION_NORMAL + 1), NULL);
}

void CatDpsDruidStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    FeralDruidStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "cat form",
        NextAction::array(0, new NextAction("cat form", 65.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "rake",
        NextAction::array(0, new NextAction("rake", ACTION_NORMAL + 5), NULL)));

    triggers.push_back(new TriggerNode(
        "ferocious bite",
        NextAction::array(0, new NextAction("ferocious bite", ACTION_HIGH + 9), NULL)));

    triggers.push_back(new TriggerNode(
        "medium threat",
        NextAction::array(0, new NextAction("cower", ACTION_EMERGENCY + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "faerie fire (feral)",
        NextAction::array(0, new NextAction("faerie fire (feral)", ACTION_HIGH), NULL)));

	triggers.push_back(new TriggerNode(
		"tiger's fury",
		NextAction::array(0, new NextAction("tiger's fury", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "behind target",
        NextAction::array(0, new NextAction("pounce", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("prowl", ACTION_EMERGENCY - 1), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("dash", ACTION_HIGH + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "player has flag",
        NextAction::array(0, new NextAction("dash", ACTION_EMERGENCY + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy flagcarrier near",
        NextAction::array(0, new NextAction("dash", 81.0f), NULL)));
}

void CatAoeDruidStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "medium aoe",
        NextAction::array(0, new NextAction("swipe (cat)", ACTION_HIGH + 2), NULL)));
}

