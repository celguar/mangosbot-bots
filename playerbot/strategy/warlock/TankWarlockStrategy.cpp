#include "botpch.h"
#include "../../playerbot.h"
#include "WarlockMultipliers.h"
#include "TankWarlockStrategy.h"

using namespace ai;

class GenericWarlockStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericWarlockStrategyActionNodeFactory()
    {
        creators["summon voidwalker"] = &summon_voidwalker;
        creators["summon felguard"] = &summon_felguard;
        creators["summon succubus"] = &summon_succubus;
    }
private:
    static ActionNode* summon_voidwalker(PlayerbotAI* ai)
    {
        return new ActionNode ("summon voidwalker",
            /*P*/ NULL,
            /*A*/ NextAction::array(0, new NextAction("summon imp"), NULL),
            /*C*/ NULL);
    }
    static ActionNode* summon_felguard(PlayerbotAI* ai)
    {
        return new ActionNode ("summon felguard",
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

TankWarlockStrategy::TankWarlockStrategy(PlayerbotAI* ai) : GenericWarlockStrategy(ai)
{
    actionNodeFactories.Add(new GenericWarlockStrategyActionNodeFactory());
}

NextAction** TankWarlockStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("shoot", 10.0f), NULL);
}

void TankWarlockStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    GenericWarlockStrategy::InitTriggers(triggers);
}
