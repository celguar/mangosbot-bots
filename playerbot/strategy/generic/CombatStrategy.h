#pragma once

namespace ai
{
    // TO DO: Remove this class when no more dependencies
    class CombatStrategy : public Strategy
    {
    public:
        CombatStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        virtual int GetType() override { return STRATEGY_TYPE_COMBAT; }

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AvoidAoeStrategy : public Strategy
    {
    public:
        AvoidAoeStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        string getName() override { return "avoid aoe"; }

#ifdef GenerateBotHelp
        virtual string GetHelpName() { return "avoid aoe"; } //Must equal iternal name
        virtual string GetHelpDescription() 
        {
            return "This strategy will make bots move away when they are in aoe.";
        }
        virtual vector<string> GetRelatedStrategies() { return { }; }
#endif

    private:
        void InitCombatMultipliers(std::list<Multiplier*>& multipliers) override;
        void InitReactionMultipliers(std::list<Multiplier*>& multipliers) override;
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AvoidAoeStrategyMultiplier : public Multiplier
    {
    public:
        AvoidAoeStrategyMultiplier(PlayerbotAI* ai) : Multiplier(ai, "run away on area debuff") {}
        float GetValue(Action* action) override;
    };

    class WaitForAttackStrategy : public Strategy
    {
    public:
        WaitForAttackStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        string getName() override { return "wait for attack"; }

        static bool ShouldWait(PlayerbotAI* ai);
        static uint8 GetWaitTime(PlayerbotAI* ai);
        static float GetSafeDistance() { return sPlayerbotAIConfig.spellDistance; }
        static float GetSafeDistanceThreshold() { return 2.5f; }

#ifdef GenerateBotHelp
        virtual string GetHelpName() { return "wait for attack"; } //Must equal iternal name
        virtual string GetHelpDescription() {
            return "This strategy will make bots wait a specified time before attacking.";
        }
        virtual vector<string> GetRelatedStrategies() { return { }; }
#endif

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatMultipliers(std::list<Multiplier*>& multipliers) override;
    };

    class WaitForAttackMultiplier : public Multiplier
    {
    public:
        WaitForAttackMultiplier(PlayerbotAI* ai) : Multiplier(ai, "wait for for attack") {}
        float GetValue(Action* action) override;
    };

    class HealInterruptStrategy : public Strategy
    {
    public:
        HealInterruptStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        string getName() override { return "heal interrupt"; }

#ifdef GenerateBotHelp
        virtual string GetHelpName() { return "heal interrupt"; } //Must equal iternal name
        virtual string GetHelpDescription()
        {
            return "This strategy will make the bot interrupt the heal it currently casts if target is at full health";
        }
#endif

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class PreHealStrategy : public Strategy
    {
    public:
        PreHealStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        string getName() override { return "preheal"; }

#ifdef GenerateBotHelp
        virtual string GetHelpName() { return "preheal"; } //Must equal iternal name
        virtual string GetHelpDescription()
        {
            return "This strategy will make the bot calculate melee damage of attacker when deciding how to heal target";
        }
#endif
    };
}
