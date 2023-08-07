#pragma once

#include "../Action.h"
#include "MovementActions.h"

namespace ai
{
	class AttackAction : public MovementAction
	{
	public:
		AttackAction(PlayerbotAI* ai, string name) : MovementAction(ai, name) {}

    public:
        virtual bool Execute(Event& event);
        virtual bool isPossible() override { return true; }; //Override movement stay.

    protected:
        bool Attack(Unit* target);
        bool IsTargetValid(Unit* target);
    };

    class AttackMyTargetAction : public AttackAction
    {
    public:
        AttackMyTargetAction(PlayerbotAI* ai, string name = "attack my target") : AttackAction(ai, name) {}

    public:
        virtual bool Execute(Event& event);
        virtual bool isUseful();
    };

    class AttackRTITargetAction : public AttackAction
    {
    public:
        AttackRTITargetAction(PlayerbotAI* ai, string name = "attack rti target") : AttackAction(ai, name) {}

    public:
        virtual bool Execute(Event& event);
        virtual bool isUseful();
    };

    class AttackDuelOpponentAction : public AttackAction
    {
    public:
        AttackDuelOpponentAction(PlayerbotAI* ai, string name = "attack duel opponent") : AttackAction(ai, name) {}

    public:
        virtual bool Execute(Event& event);
        virtual bool isUseful();
    };
}
