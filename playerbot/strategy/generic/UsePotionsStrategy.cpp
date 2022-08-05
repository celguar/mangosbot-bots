#include "botpch.h"
#include "../../playerbot.h"
#include "UsePotionsStrategy.h"

using namespace ai;

class UsePotionsStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    UsePotionsStrategyActionNodeFactory()
    {
        creators["healthstone"] = &healthstone;
    }
private:
    static ActionNode* healthstone(PlayerbotAI* ai)
    {
        return new ActionNode("healthstone",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("healing potion"), NULL),
            /*C*/ NULL);
    }
};

UsePotionsStrategy::UsePotionsStrategy(PlayerbotAI* ai) : Strategy(ai)
{
    actionNodeFactories.Add(new UsePotionsStrategyActionNodeFactory());
}

void UsePotionsStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    Strategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("healthstone", ACTION_EMERGENCY + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0,  new NextAction("mana potion", ACTION_EMERGENCY + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("dark rune", ACTION_EMERGENCY), NULL)));
}
