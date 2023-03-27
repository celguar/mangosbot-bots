#pragma once

#include "../Action.h"
#include "MovementActions.h"
#include "../../PlayerbotAIConfig.h"
#include "../../ServerFacade.h"
#include "../generic/PullStrategy.h"
#include "../NamedObjectContext.h"
#include "GenericSpellActions.h"

namespace ai
{
    class ReachTargetAction : public MovementAction, public Qualified
    {
    public:
        ReachTargetAction(PlayerbotAI* ai, string name, float range = 0.0f) : MovementAction(ai, name), Qualified(), range(range), spellName("") {}

        void Qualify(string qualifier)
        {
            Qualified::Qualify(qualifier);

            // Get the distance from the qualified spell given
            if (!qualifier.empty())
            {
                spellName = Qualified::getMultiQualifierStr(qualifier, 0, "::");

                float maxSpellRange;
                if (ai->GetSpellRange(spellName, &maxSpellRange))
                {
                    range = maxSpellRange;
                }
            }
        }

        virtual bool Execute(Event& event)
		{
            Unit* target = GetTarget();
            if (target)
            {
                UpdateMovementState();

                // Ignore movement if too far
                const float distanceToTarget = bot->GetDistance(target, false, DIST_CALC_COMBAT_REACH);
                float chaseDist = range;
                const bool inLos = bot->IsWithinLOSInMap(target, true);
                const bool isFriend = sServerFacade.IsFriendlyTo(bot, target);

                if (range > 0.0f)
                {
                    chaseDist = inLos ? range : (isFriend ? std::min(distanceToTarget * 0.9f, range) : range);
                    chaseDist = (chaseDist - sPlayerbotAIConfig.contactDistance);
                }

                if (inLos && isFriend && (range <= ai->GetRange("follow")))
                {
                    return MoveNear(target, chaseDist);
                }
                else
                {
                    return ChaseTo(target, chaseDist, bot->GetAngle(target));
                }
            }

            return false;
        }

        virtual bool isUseful()
		{
            Unit* target = GetTarget();
            if (!target)
                return false;

            // Do not move while casting
            if (bot->IsNonMeleeSpellCasted(true))
                return false;

            // Do not move if stay strategy is set
            if(ai->HasStrategy("stay", ai->GetState()))
                return false;

            // Check if the spell for which the reach action is used for can be casted
            if (!spellName.empty())
            {
                if (!ai->CanCastSpell(spellName, target, true, nullptr, true))
                {
                    return false;
                }
            }

            // Check if the bot is already on the range required
            return bot->GetDistance(target, true, DIST_CALC_COMBAT_REACH) > range;
        }

        virtual string GetTargetName() { return "current target"; }
        string GetSpellName() const { return spellName; }

        virtual Unit* GetTarget() override
        {
            // Get the target from the qualifiers
            if (!qualifier.empty())
            {
                const string targetName = Qualified::getMultiQualifierStr(qualifier, 1, "::");
                const string targetQualifier = Qualified::getMultiQualifierStr(qualifier, 2, "::");
                return targetQualifier.empty() ? AI_VALUE(Unit*, targetName) : AI_VALUE2(Unit*, targetName, targetQualifier);
            }
            else
            {
                return AI_VALUE(Unit*, GetTargetName());
            }
        }

    protected:
        float range;
        string spellName;
    };

    class CastReachTargetSpellAction : public CastSpellAction
    {
    public:
        CastReachTargetSpellAction(PlayerbotAI* ai, string spell, float distance) : CastSpellAction(ai, spell), distance(distance) {}

		virtual bool isUseful()
		{
            // Do not move if stay strategy is set
            if (ai->HasStrategy("stay", ai->GetState()))
                return false;

			return sServerFacade.IsDistanceGreaterThan(AI_VALUE2(float, "distance", "current target"), (distance + sPlayerbotAIConfig.contactDistance));
		}

    protected:
        float distance;
    };

    class ReachMeleeAction : public ReachTargetAction
	{
    public:
        ReachMeleeAction(PlayerbotAI* ai) : ReachTargetAction(ai, "reach melee") {}
    };

    class ReachSpellAction : public ReachTargetAction
	{
    public:
        ReachSpellAction(PlayerbotAI* ai) : ReachTargetAction(ai, "reach spell", ai->GetRange("spell")) {}
    };

    class ReachPullAction : public ReachTargetAction
    {
    public:
        ReachPullAction(PlayerbotAI* ai) : ReachTargetAction(ai, "reach pull")
        {
            PullStrategy* strategy = PullStrategy::Get(ai);
            if (strategy)
            {
                range = strategy->GetRange();
            }
        }

        string GetTargetName() override { return "pull target"; }
    };

    class ReachPartyMemberToHealAction : public ReachTargetAction
    {
    public:
        ReachPartyMemberToHealAction(PlayerbotAI* ai) : ReachTargetAction(ai, "reach party member to heal", ai->GetRange("heal")) {}
        virtual string GetTargetName() { return "party member to heal"; }
    };
}
