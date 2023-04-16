#include "botpch.h"
#include "../../playerbot.h"
#include "MageMultipliers.h"
#include "GenericMageNonCombatStrategy.h"

using namespace ai;

class GenericMageNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericMageNonCombatStrategyActionNodeFactory()
    {
        creators["molten armor"] = &molten_armor;
        creators["mage armor"] = &mage_armor;
        creators["ice armor"] = &ice_armor;
    }
private:
    static ActionNode* molten_armor(PlayerbotAI* ai)
    {
        return new ActionNode ("molten armor",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("mage armor"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* mage_armor(PlayerbotAI* ai)
    {
        return new ActionNode ("mage armor",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("ice armor"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* ice_armor(PlayerbotAI* ai)
    {
        return new ActionNode ("ice armor",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("frost armor"), NULL),
            /*C*/ NULL);
    }
};

GenericMageNonCombatStrategy::GenericMageNonCombatStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai)
{
    actionNodeFactories.Add(new GenericMageNonCombatStrategyActionNodeFactory());
}

void GenericMageNonCombatStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    NonCombatStrategy::InitNonCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "arcane intellect",
        NextAction::array(0, new NextAction("arcane intellect", 21.0f), NULL)));

	triggers.push_back(new TriggerNode(
		"no drink",
		NextAction::array(0, new NextAction("conjure water", 16.0f), NULL)));

	triggers.push_back(new TriggerNode(
		"no food",
		NextAction::array(0, new NextAction("conjure food", 15.0f), NULL)));

   triggers.push_back(new TriggerNode(
      "often",
      NextAction::array(0, new NextAction("apply oil", 1.0f), NULL)));

#ifdef MANGOSBOT_TWO
   triggers.push_back(new TriggerNode(
       "learn glyph of fireball",
       NextAction::array(0, new NextAction("learn glyph of fireball", 20.0f), NULL)));

   triggers.push_back(new TriggerNode(
       "remove glyph of fireball",
       NextAction::array(0, new NextAction("remove glyph of fireball", 20.0f), NULL)));

   triggers.push_back(new TriggerNode(
       "learn glyph of living bomb",
       NextAction::array(0, new NextAction("learn glyph of living bomb", 20.0f), NULL)));

   triggers.push_back(new TriggerNode(
       "remove glyph of living bomb",
       NextAction::array(0, new NextAction("remove glyph of living bomb", 20.0f), NULL)));

   triggers.push_back(new TriggerNode(
       "learn glyph of molten armor",
       NextAction::array(0, new NextAction("learn glyph of molten armor", 20.0f), NULL)));

   triggers.push_back(new TriggerNode(
       "remove glyph of molten armor",
       NextAction::array(0, new NextAction("remove glyph of molten armor", 20.0f), NULL)));
#endif
}

void MageBuffManaStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "mage armor",
        NextAction::array(0, new NextAction("mage armor", 18.0f), NULL)));
}

void MageBuffManaStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    InitNonCombatTriggers(triggers);
}

void MageBuffDpsStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "mage armor",
        NextAction::array(0, new NextAction("molten armor", 19.0f), NULL)));
}

void MageBuffDpsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    InitNonCombatTriggers(triggers);
}

void MageBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "arcane intellect on party",
        NextAction::array(0, new NextAction("arcane intellect on party", 20.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "give water",
        NextAction::array(0, new NextAction("give water", 14.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "give food",
        NextAction::array(0, new NextAction("give food", 13.0f), NULL)));
}