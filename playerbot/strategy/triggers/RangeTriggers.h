#pragma once
#include "../Trigger.h"
#include "../../PlayerbotAIConfig.h"
#include "../../ServerFacade.h"

namespace ai
{
    class EnemyTooCloseForSpellTrigger : public Trigger {
    public:
        EnemyTooCloseForSpellTrigger(PlayerbotAI* ai) : Trigger(ai, "enemy too close for spell") {}
        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, "current target");
            if (target)
            {
                // Don't move if the target is targeting you and you can't add distance between you and the target
                if (target->GetTarget() == bot && !target->IsRooted() && target->GetSpeedInMotion() > (bot->GetSpeedInMotion() * 0.65))
                {
                    return false;
                }

                float const combatReach = bot->GetCombinedCombatReach(target, false);
                float const minDistance = ai->GetRange("spell") + combatReach;
                float const targetDistance = sServerFacade.GetDistance2d(bot, target) + combatReach;

                // No need to move if the target is rooted and you can shoot
                if (target->IsRooted() && (targetDistance > minDistance))
                {
                    return false;
                }

                bool isBoss = false;
                bool isRaid = false;
                bool isVictim = target->GetVictim() && target->GetVictim()->GetObjectGuid() == bot->GetObjectGuid();

                if (target->IsCreature())
                {
                    Creature* creature = ai->GetCreature(target->GetObjectGuid());
                    if (creature)
                    {
                        isBoss = creature->IsWorldBoss();
                    }
                }

                if (bot->GetMap() && bot->GetMap()->IsRaid())
                    isRaid = true;

                //if (isBoss || isRaid)
                //    return sServerFacade.IsDistanceLessThan(targetDistance, (ai->GetRange("spell") + combatReach) / 2);

                float coeff = 0.5f;
                if (target->IsPlayer())
                {
                    if (!isVictim)
                        coeff = 0.7f;
                    else
                        coeff = 1.0f;
                }
                else
                {
                    if (!isVictim)
                        coeff = 0.4f;
                    else
                        coeff = 0.6f;
                }

                if (isRaid)
                    coeff = 1.0f;

                return sServerFacade.IsDistanceLessOrEqualThan(targetDistance, minDistance * coeff);
            }
            return false;
        }
    };

    class EnemyTooCloseForShootTrigger : public Trigger {
    public:
        EnemyTooCloseForShootTrigger(PlayerbotAI* ai) : Trigger(ai, "enemy too close for shoot") {}
        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, "current target");
            if (target)
            {
                // Don't move if the target is targeting you and you can't add distance between you and the target
                if (target->GetTarget() == bot && !target->IsRooted() && target->GetSpeedInMotion() > (bot->GetSpeedInMotion() * 0.65))
                {
                    return false;
                }

                float const combatReach = bot->GetCombinedCombatReach(target, false);
                float const minShootDistance = ai->GetRange("shoot") + combatReach;
                float const targetDistance = sServerFacade.GetDistance2d(bot, target) + combatReach;

                // No need to move if the target is rooted and you can shoot
                if (target->IsRooted() && (targetDistance > minShootDistance))
                {
                    return false;
                }

                bool isBoss = false;
                bool isRaid = false;
                bool isVictim = target->GetVictim() && target->GetVictim()->GetObjectGuid() == bot->GetObjectGuid();

                if (target->IsCreature())
                {
                    Creature* creature = ai->GetCreature(target->GetObjectGuid());
                    if (creature)
                    {
                        isBoss = creature->IsWorldBoss();
                    }
                }

                if (bot->GetMap() && bot->GetMap()->IsRaid())
                    isRaid = true;

                //if (isBoss || isRaid)
                //    return sServerFacade.IsDistanceLessThan(targetDistance, (ai->GetRange("spell") + combatReach));

                float coeff = 0.5f;
                if (target->IsPlayer())
                {
                    if (!isVictim)
                        coeff = 0.7f;
                    else
                        coeff = 1.0f;
                }
                else
                {
                    if (!isVictim)
                        coeff = 0.4f;
                    else
                        coeff = 0.6f;
                }

                if (isRaid)
                    coeff = 1.0f;

                return sServerFacade.IsDistanceLessOrEqualThan(targetDistance, minShootDistance * coeff);
            }
            return false;
        }
    };

    class EnemyTooCloseForMeleeTrigger : public Trigger {
    public:
        EnemyTooCloseForMeleeTrigger(PlayerbotAI* ai) : Trigger(ai, "enemy too close for melee", 3) {}
        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, "current target");
            if (target && target->IsPlayer())
                return false;

            return target && AI_VALUE2(bool, "inside target", "current target");
        }
    };

    class EnemyIsCloseTrigger : public Trigger {
    public:
        EnemyIsCloseTrigger(PlayerbotAI* ai) : Trigger(ai, "enemy is close") {}
        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, "current target");
            return target &&
                sServerFacade.IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", "current target"), sPlayerbotAIConfig.tooCloseDistance);
        }
    };

    class EnemyInRangeTrigger : public Trigger {
    public:
        EnemyInRangeTrigger(PlayerbotAI* ai, string name, float distance) : Trigger(ai, name)
        {
            this->distance = distance;
        }
        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, "current target");
            return target &&
                sServerFacade.IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", "current target"), distance);
        }
    protected:
        float distance;
    };

    class OutOfRangeTrigger : public Trigger {
    public:
        OutOfRangeTrigger(PlayerbotAI* ai, string name, float distance) : Trigger(ai, name)
        {
            this->distance = distance;
        }
        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, GetTargetName());
            return target &&
                sServerFacade.IsDistanceGreaterThan(AI_VALUE2(float, "distance", GetTargetName()), distance);
        }
        virtual string GetTargetName() { return "current target"; }

    protected:
        float distance;
    };

    class EnemyOutOfMeleeTrigger : public OutOfRangeTrigger
    {
    public:
        EnemyOutOfMeleeTrigger(PlayerbotAI* ai) : OutOfRangeTrigger(ai, "enemy out of melee range", sPlayerbotAIConfig.meleeDistance) {}
        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, GetTargetName());
            if (!target)
                return false;

            return !bot->CanReachWithMeleeAttack(target) || !bot->IsWithinLOSInMap(target, true);
        }
    };

    class EnemyOutOfSpellRangeTrigger : public OutOfRangeTrigger
    {
    public:
        EnemyOutOfSpellRangeTrigger(PlayerbotAI* ai) : OutOfRangeTrigger(ai, "enemy out of spell range", ai->GetRange("spell")) {}
        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, GetTargetName());
            if (!target)
                return false;

            return target && (bot->GetDistance(target, true, DIST_CALC_COMBAT_REACH) > (distance - sPlayerbotAIConfig.contactDistance)) || !bot->IsWithinLOSInMap(target, true);
        }
    };

    class PartyMemberToHealOutOfSpellRangeTrigger : public OutOfRangeTrigger
    {
    public:
        PartyMemberToHealOutOfSpellRangeTrigger(PlayerbotAI* ai) : OutOfRangeTrigger(ai, "party member to heal out of spell range", ai->GetRange("heal")) {}
        virtual string GetTargetName() { return "party member to heal"; }
        virtual bool IsActive()
        {
            Unit* target = AI_VALUE(Unit*, GetTargetName());
            if (!target)
                return false;

            return target && (bot->GetDistance(target, true, DIST_CALC_COMBAT_REACH) > (distance - sPlayerbotAIConfig.contactDistance)) || !bot->IsWithinLOSInMap(target, true);
        }
    };

    class FarFromMasterTrigger : public Trigger {
    public:
        FarFromMasterTrigger(PlayerbotAI* ai, string name = "far from master", float distance = 12.0f, int checkInterval = 50) : Trigger(ai, name, checkInterval), distance(distance) {}

        virtual bool IsActive()
        {
            return sServerFacade.IsDistanceGreaterThan(AI_VALUE2(float, "distance", "master target"), distance);
        }

    private:
        float distance;
    };

    class OutOfReactRangeTrigger : public FarFromMasterTrigger
    {
    public:
        OutOfReactRangeTrigger(PlayerbotAI* ai) : FarFromMasterTrigger(ai, "out of react range", 50.0f, 5) {}
    };

    class NotNearMasterTrigger : public FarFromMasterTrigger
    {
    public:
        NotNearMasterTrigger(PlayerbotAI* ai) : FarFromMasterTrigger(ai, "not near master", 5.0f, 2) {}
    };
}