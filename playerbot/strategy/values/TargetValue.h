#pragma once
#include "../Value.h"
#include "../../TravelMgr.h"

namespace ai
{
    class FindTargetStrategy
    {
    public:
        FindTargetStrategy(PlayerbotAI* ai)
        {
            result = NULL;
            this->ai = ai;
        }

    public:
        Unit* GetResult() { return result; }

    public:
        virtual void CheckAttacker(Unit* attacker, ThreatManager* threatManager) = 0;
        void GetPlayerCount(Unit* creature, int* tankCount, int* dpsCount);

    protected:
        Unit* result;
        PlayerbotAI* ai;

    protected:
        map<Unit*, int> tankCountCache;
        map<Unit*, int> dpsCountCache;
    };

    class FindNonCcTargetStrategy : public FindTargetStrategy
    {
    public:
        FindNonCcTargetStrategy(PlayerbotAI* ai) : FindTargetStrategy(ai) {}

    protected:
        virtual bool IsCcTarget(Unit* attacker);

    };

    class TargetValue : public UnitCalculatedValue
	{
	public:
        TargetValue(PlayerbotAI* ai, string name = "target", int checkInterval = 1) : UnitCalculatedValue(ai, name, checkInterval) {}

    protected:
        Unit* FindTarget(FindTargetStrategy* strategy);
    };

    class RpgTargetValue : public ManualSetValue<GuidPosition>
    {
    public:
        RpgTargetValue(PlayerbotAI* ai, string name = "rpg target") : ManualSetValue<GuidPosition>(ai, GuidPosition(), name) {}
    };

    class TravelTargetValue : public ManualSetValue<TravelTarget *>
    {
    public:
        TravelTargetValue(PlayerbotAI* ai, string name = "travel target") : ManualSetValue<TravelTarget*>(ai, new TravelTarget(ai), name) {}
        virtual ~TravelTargetValue() { delete value; }
    };	

    class LastLongMoveValue : public CalculatedValue<WorldPosition>
    {
    public:
        LastLongMoveValue(PlayerbotAI* ai) : CalculatedValue<WorldPosition>(ai, "last long move", 30) {}

        WorldPosition Calculate();
    };

    class HomeBindValue : public CalculatedValue<WorldPosition>
    {
    public:
        HomeBindValue(PlayerbotAI* ai) : CalculatedValue<WorldPosition>(ai, "home bind", 30) {}

        WorldPosition Calculate();
    };

    class IgnoreRpgTargetValue : public ManualSetValue<set<ObjectGuid>& >
    {
    public:
        IgnoreRpgTargetValue(PlayerbotAI* ai) : ManualSetValue<set<ObjectGuid>& >(ai, data, "ignore rpg targets") {}

    private:
        set<ObjectGuid> data;
    };

    class TalkTargetValue : public ManualSetValue<ObjectGuid>
    {
    public:
        TalkTargetValue(PlayerbotAI* ai, string name = "talk target") : ManualSetValue<ObjectGuid>(ai, ObjectGuid(), name) {}
    };

    class AttackTargetValue : public ManualSetValue<ObjectGuid>
    {
    public:
        AttackTargetValue(PlayerbotAI* ai, string name = "attack target") : ManualSetValue<ObjectGuid>(ai, ObjectGuid(), name) {}
    };

    class PullTargetValue : public UnitManualSetValue
    {
    public:
        PullTargetValue(PlayerbotAI* ai, string name = "pull target") : UnitManualSetValue(ai, NULL, name) {}
        void Set(Unit* unit) override;
        Unit* Get() override;

    private:
        ObjectGuid guid;
    };
}
