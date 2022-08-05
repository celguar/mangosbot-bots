#include "botpch.h"
#include "../../playerbot.h"
#include "WarriorMultipliers.h"
#include "TankWarriorStrategy.h"
#include "WarriorTriggers.h"

using namespace ai;

class TankWarriorStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    TankWarriorStrategyActionNodeFactory()
    {
        creators["charge"] = &charge;
        creators["sunder armor"] = &sunder_armor;
        creators["commanding shout"] = &commanding_shout;
        creators["shield slam"] = &shield_slam;
        creators["devastate"] = &devastate;
        creators["last stand"] = &last_stand;
        creators["heroic throw on snare target"] = &heroic_throw_on_snare_target;
        creators["heroic throw taunt"] = &heroic_throw_taunt;
        creators["taunt"] = &taunt;
    }
private:
    //ACTION_NODE_A(charge, "charge", "intercept with stance");
    ACTION_NODE_A(charge, "charge", "reach melee");
    ACTION_NODE_A(sunder_armor, "sunder armor", "melee");
    ACTION_NODE_A(commanding_shout, "commanding shout", "battle shout");
    ACTION_NODE_A(shield_slam, "shield slam", "heroic strike");
    ACTION_NODE_A(devastate, "devastate", "sunder armor");
    ACTION_NODE_A(last_stand, "last stand", "intimidating shout");
    ACTION_NODE_A(heroic_throw_on_snare_target, "heroic throw on snare target", "taunt on snare target");
    ACTION_NODE_A(heroic_throw_taunt, "heroic throw", "taunt");
    ACTION_NODE_A(taunt, "taunt", "battle shout taunt");
};

TankWarriorStrategy::TankWarriorStrategy(PlayerbotAI* ai) : GenericWarriorStrategy(ai)
{
    actionNodeFactories.Add(new TankWarriorStrategyActionNodeFactory());
}

NextAction** TankWarriorStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("melee", ACTION_NORMAL), NULL);
}

void TankWarriorStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    GenericWarriorStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("heroic throw", ACTION_MOVE + 12), new NextAction("charge", ACTION_MOVE + 11), NULL)));

    triggers.push_back(new TriggerNode(
        "intercept and rage",
        NextAction::array(0, new NextAction("berserker stance", ACTION_INTERRUPT + 14), NULL)));

    triggers.push_back(new TriggerNode(
        "intercept and rage",
        NextAction::array(0, new NextAction("intercept", ACTION_INTERRUPT + 13), NULL)));

#ifdef MANGOSBOT_ZERO
    triggers.push_back(new TriggerNode(
        "thunder clap and rage and aoe",
        NextAction::array(0, new NextAction("battle stance", ACTION_INTERRUPT + 12), NULL)));

    triggers.push_back(new TriggerNode(
        "thunder clap and rage and aoe",
        NextAction::array(0, new NextAction("thunder clap", ACTION_INTERRUPT + 11), NULL)));
#endif

    triggers.push_back(new TriggerNode(
        "defensive stance",
        NextAction::array(0, new NextAction("defensive stance", ACTION_MOVE + 11), NULL)));

    triggers.push_back(new TriggerNode(
        "commanding shout",
        NextAction::array(0, new NextAction("commanding shout", ACTION_HIGH + 8), NULL)));

    triggers.push_back(new TriggerNode(
        "bloodrage",
        NextAction::array(0, new NextAction("bloodrage", ACTION_INTERRUPT + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "sunder armor",
        NextAction::array(0, new NextAction("devastate", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "medium rage available",
        NextAction::array(0, new NextAction("shield slam", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "shield block",
        NextAction::array(0, new NextAction("shield block", ACTION_INTERRUPT + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "revenge",
        NextAction::array(0, new NextAction("revenge", ACTION_HIGH + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "disarm",
        NextAction::array(0, new NextAction("disarm", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "lose aggro",
        NextAction::array(0, new NextAction("heroic throw taunt", ACTION_INTERRUPT + 10), NULL)));

    triggers.push_back(new TriggerNode(
        "taunt on snare target",
        NextAction::array(0, new NextAction("heroic throw on snare target", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("shield wall", ACTION_MEDIUM_HEAL), NULL)));

	triggers.push_back(new TriggerNode(
		"critical health",
		NextAction::array(0, new NextAction("last stand", ACTION_EMERGENCY + 3), NULL)));

	triggers.push_back(new TriggerNode(
        "medium aoe",
        NextAction::array(0, new NextAction("battle shout taunt", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "medium aoe",
        NextAction::array(0, new NextAction("challenging shout", ACTION_HIGH + 3), NULL)));

	triggers.push_back(new TriggerNode(
		"concussion blow",
		NextAction::array(0, new NextAction("concussion blow", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "shield bash",
        NextAction::array(0, new NextAction("shield bash", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "shield bash on enemy healer",
        NextAction::array(0, new NextAction("shield bash on enemy healer", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "spell reflection",
        NextAction::array(0, new NextAction("spell reflection", ACTION_INTERRUPT + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "victory rush",
        NextAction::array(0, new NextAction("victory rush", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "sword and board",
        NextAction::array(0, new NextAction("shield slam", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "rend",
        NextAction::array(0, new NextAction("rend", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "protect party member",
        NextAction::array(0, new NextAction("intervene", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "light aoe",
        NextAction::array(0, new NextAction("goblin sapper", ACTION_HIGH + 9), NULL)));

    triggers.push_back(new TriggerNode(
        "light aoe",
        NextAction::array(0, new NextAction("oil of immolation", ACTION_HIGH + 8), NULL)));
}
