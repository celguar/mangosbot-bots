#include "botpch.h"
#include "../../playerbot.h"
#include "MageMultipliers.h"
#include "GenericMageStrategy.h"

using namespace ai;

class GenericMageStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericMageStrategyActionNodeFactory()
    {
        creators["frostbolt"] = &frostbolt;
        creators["fire blast"] = &fire_blast;
        creators["scorch"] = &scorch;
        creators["frost nova"] = &frost_nova;
        creators["cone of cold"] = &cone_of_cold;
        creators["combustion"] = &combustion;
        creators["evocation"] = &evocation;
        creators["remove curse"] = &remove_curse;
        creators["remove curse on party"] = &remove_curse_on_party;
#ifdef MANGOSBOT_TWO
        creators["frostfire bolt"] = &frostfire_bolt;
#endif
#ifndef MANGOSBOT_ZERO
        creators["dragon's breath"] = &dragons_breath;
        creators["blast wave"] = &blast_wave;
#endif
    }
private:
    static ActionNode* frostbolt(PlayerbotAI* ai)
    {
        return new ActionNode ("frostbolt",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("shoot"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* fire_blast(PlayerbotAI* ai)
    {
        return new ActionNode ("fire blast",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("scorch"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* scorch(PlayerbotAI* ai)
    {
        return new ActionNode ("scorch",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("shoot"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* frost_nova(PlayerbotAI* ai)
    {
        return new ActionNode("frost nova",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("cone of cold"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* cone_of_cold(PlayerbotAI* ai)
    {
        return new ActionNode("cone of cold",
            /*P*/ NULL,
            /*A*/ NULL,
            /*C*/ NULL);
    }
    static ActionNode* combustion(PlayerbotAI* ai)
    {
        return new ActionNode ("combustion",
            /*P*/ NULL,
            /*A*/ NULL,
            /*C*/ NULL);
    }
    static ActionNode* evocation(PlayerbotAI* ai)
    {
        return new ActionNode ("evocation",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("mana potion"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* remove_curse(PlayerbotAI* ai)
    {
        return new ActionNode ("remove curse",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("remove lesser curse"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* remove_curse_on_party(PlayerbotAI* ai)
    {
        return new ActionNode ("remove curse on party",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("remove lesser curse on party"), NULL),
            /*C*/ NULL);
    }
#ifdef MANGOSBOT_TWO
    static ActionNode* frostfire_bolt(PlayerbotAI* ai)
    {
        return new ActionNode("frostfire bolt",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("fireball"), NULL),
            /*C*/ NULL);
    }
#endif
#ifndef MANGOSBOT_ZERO
    static ActionNode* dragons_breath(PlayerbotAI* ai)
    {
        return new ActionNode("dragon's breath",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("blast wave"), NULL),
            /*C*/ NextAction::array(0, new NextAction("flamestrike", 71.0f), NULL));
    }
    static ActionNode* blast_wave(PlayerbotAI* ai)
    {
        return new ActionNode("blast wave",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("frost nova"), NULL),
            /*C*/ NextAction::array(0, new NextAction("flamestrike", 71.0f), NULL));
    }
#endif
};

GenericMageStrategy::GenericMageStrategy(PlayerbotAI* ai) : CombatStrategy(ai)
{
    actionNodeFactories.Add(new GenericMageStrategyActionNodeFactory());
}

void GenericMageStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    CombatStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy ten yards",
        NextAction::array(0, new NextAction("frost nova", 61.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "counterspell on enemy healer",
        NextAction::array(0, new NextAction("counterspell on enemy healer", 40.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "counterspell",
        NextAction::array(0, new NextAction("counterspell", 40.0f), NULL)));

	triggers.push_back(new TriggerNode(
		"critical health",
		NextAction::array(0, new NextAction("ice block", 80.0f), NULL)));

	triggers.push_back(new TriggerNode(
		"spellsteal",
		NextAction::array(0, new NextAction("spellsteal", 40.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "medium threat",
        NextAction::array(0, new NextAction("invisibility", 40.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("evocation", ACTION_HIGH - 1), NULL)));

    triggers.push_back(new TriggerNode(
        "fire ward",
        NextAction::array(0, new NextAction("fire ward", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "frost ward",
        NextAction::array(0, new NextAction("frost ward", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "blink",
        NextAction::array(0, new NextAction("blink", 61.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "target critical health",
        NextAction::array(0, new NextAction("fire blast", ACTION_HIGH), NULL)));
}

MageCureStrategy::MageCureStrategy(PlayerbotAI* ai) : Strategy(ai)
{
    actionNodeFactories.Add(new GenericMageStrategyActionNodeFactory());
}

void MageCureStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "remove curse",
        NextAction::array(0, new NextAction("remove curse", ACTION_DISPEL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "remove curse on party",
        NextAction::array(0, new NextAction("remove curse on party", ACTION_DISPEL), NULL)));
}

void MageCureStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    InitCombatTriggers(triggers);
}

void MageBoostStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "arcane power",
        NextAction::array(0, new NextAction("arcane power", 41.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "presence of mind",
        NextAction::array(0, new NextAction("presence of mind", 42.0f), NULL)));

#ifdef MANGOSBOT_TWO
    triggers.push_back(new TriggerNode(
        "mirror image",
        NextAction::array(0, new NextAction("mirror image", 59.0f), NULL)));
#endif
}

void MageCcStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "polymorph",
        NextAction::array(0, new NextAction("polymorph", 30.0f), NULL)));
}
