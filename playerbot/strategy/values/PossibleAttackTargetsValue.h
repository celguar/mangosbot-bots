#pragma once
#include "../Value.h"
#include "TargetValue.h"
#include "NearestUnitsValue.h"

namespace ai
{
    // List of hostile targets that are in combat with the bot (or bot group) and can be attacked (is reachable, not cc'ed, etc...)
    class PossibleAttackTargetsValue : public ObjectGuidListCalculatedValue, public Qualified
	{
	public:
        PossibleAttackTargetsValue(PlayerbotAI* ai) : ObjectGuidListCalculatedValue(ai, "possible attack targets", 2), Qualified() {}
        list<ObjectGuid> Calculate();
        static bool IsValid(Unit* target, Player* player, float range = sPlayerbotAIConfig.sightDistance, bool ignoreCC = false, bool checkAttackerValid = true);
        static bool IsPossibleTarget(Unit* target, Player* player, float range, bool ignoreCC);
        static bool HasBreakableCC(Unit* target, Player* player);
        static bool HasUnBreakableCC(Unit* target, Player* player);
        virtual string Format();

#ifdef GenerateBotHelp
        virtual string GetHelpName() { return "possible attack targets"; } //Must equal iternal name
        virtual string GetHelpTypeName() { return "combat"; }
        virtual string GetHelpDescription()
        {
            return "This value contains all the units the bot should attack.";
        }
        virtual vector<string> GetUsedValues() { return { "attackers", "attack target"}; }
#endif 

	private:
		void RemoveNonThreating(list<ObjectGuid>& targets, bool getOne);

        static bool IsImmuneToDamage(Unit* target, Player* player);
        static bool HasIgnoreCCRti(Unit* target, Player* player);
        static bool IsTapped(Unit* target, Player* player);
    };

    class PossibleAddsValue : public BoolCalculatedValue
    {
    public:
        PossibleAddsValue(PlayerbotAI* const ai, string name = "possible adds") : BoolCalculatedValue(ai, name) {}
        virtual bool Calculate();

#ifdef GenerateBotHelp
        virtual string GetHelpName() { return "possible adds"; } //Must equal iternal name
        virtual string GetHelpTypeName() { return "combat"; }
        virtual string GetHelpDescription()
        {
            return "This value returns true if any of the targets the bot wants to attack could agro adds.";
        }
        virtual vector<string> GetUsedValues() { return { "possible targets no los", "possible attack targets" }; }
#endif 
    };
}
