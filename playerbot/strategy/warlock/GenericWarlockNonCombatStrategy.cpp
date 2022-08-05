#include "botpch.h"
#include "../../playerbot.h"
#include "WarlockMultipliers.h"
#include "GenericWarlockNonCombatStrategy.h"

using namespace ai;

class GenericWarlockNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericWarlockNonCombatStrategyActionNodeFactory()
    {
        creators["fel armor"] = &fel_armor;
        creators["demon armor"] = &demon_armor;
        creators["summon voidwalker"] = &summon_voidwalker;
        creators["summon felguard"] = &summon_felguard;
        creators["summon succubus"] = &summon_succubus;
    }
private:
    static ActionNode* fel_armor(PlayerbotAI* ai)
    {
        return new ActionNode ("fel armor",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("demon armor"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* demon_armor(PlayerbotAI* ai)
    {
        return new ActionNode ("demon armor",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("demon skin"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* summon_voidwalker(PlayerbotAI* ai)
    {
        return new ActionNode("summon voidwalker",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("summon imp"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* summon_felguard(PlayerbotAI* ai)
    {
        return new ActionNode("summon felguard",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("summon succubus"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* summon_succubus(PlayerbotAI* ai)
    {
        return new ActionNode("summon succubus",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("summon voidwalker"), NULL),
            /*C*/ NULL);
    }
};

GenericWarlockNonCombatStrategy::GenericWarlockNonCombatStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai)
{
    actionNodeFactories.Add(new GenericWarlockNonCombatStrategyActionNodeFactory());
}

void GenericWarlockNonCombatStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "demon armor",
        NextAction::array(0, new NextAction("fel armor", 21.0f), NULL)));

    /*triggers.push_back(new TriggerNode(
		"no healthstone",
		NextAction::array(0, new NextAction("create healthstone", 15.0f), NULL)));

	triggers.push_back(new TriggerNode(
		"no firestone",
		NextAction::array(0, new NextAction("create firestone", 14.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "no spellstone",
        NextAction::array(0, new NextAction("create spellstone", 13.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "spellstone",
        NextAction::array(0, new NextAction("spellstone", 13.0f), NULL)));*/
    // TODO Fix Spellstone / Firestone
}

void WarlockPetStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array(0, new NextAction("summon felguard", 60.0f), NULL)));
    // TODO Warlock pets

    triggers.push_back(new TriggerNode(
       "often",
       NextAction::array(0, new NextAction("apply oil", 1.0f), NULL)));
}
