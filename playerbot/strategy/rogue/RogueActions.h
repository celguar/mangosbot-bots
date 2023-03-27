#pragma once

#include "../actions/GenericActions.h"
#include "RogueComboActions.h"
#include "RogueOpeningActions.h"
#include "RogueFinishingActions.h"
#include "../../AiFactory.h"

namespace ai
{
    BUFF_ACTION(CastColdBloodAction, "cold blood");
    BUFF_ACTION_U(CastPreparationAction, "preparation", !bot->IsSpellReady(14177) || !bot->IsSpellReady(2983) || !bot->IsSpellReady(2094));

    class CastShadowstepAction : public CastSpellAction 
    {
    public:
        CastShadowstepAction(PlayerbotAI* ai) : CastSpellAction(ai, "shadowstep") {}
        virtual bool isPossible() { return true; }
        virtual bool isUseful() {
            return bot->HasSpell(36554) && bot->IsSpellReady(36554);
        }
        virtual bool Execute(Event& event) {
            return bot->CastSpell(GetTarget(), 36554, TRIGGERED_OLD_TRIGGERED);
        }
    };

	class CastEvasionAction : public CastBuffSpellAction
	{
	public:
		CastEvasionAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "evasion") {}
	};

	class CastSprintAction : public CastBuffSpellAction
	{
	public:
		CastSprintAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "sprint") {}
        virtual string GetTargetName() { return "self target"; }
	};

    class CastStealthAction : public CastBuffSpellAction
    {
    public:
        CastStealthAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "stealth") {}
        virtual string GetTargetName() { return "self target"; }
        virtual bool isUseful()
        {
            bool hasStealth = ai->HasAura("stealth", bot);
            if (hasStealth)
                return false;

            // do not use with WSG flag
            return !ai->HasAura(23333, bot) && !ai->HasAura(23335, bot) && !ai->HasAura(34976, bot);
        }
        virtual bool Execute(Event& event)
        {
            if (ai->CastSpell("stealth", bot))
            {
                ai->ChangeStrategy("+stealthed", BotState::BOT_STATE_COMBAT);
                bot->InterruptSpell(CURRENT_MELEE_SPELL);
            }
            return true;
        }
    };

    class UnstealthAction : public Action 
    {
    public:
        UnstealthAction(PlayerbotAI* ai) : Action(ai, "unstealth") {}
        virtual bool Execute(Event& event) {
            ai->RemoveAura("stealth");
            return true;
        }
    };

    class CheckStealthAction : public Action 
    {
    public:
        CheckStealthAction(PlayerbotAI* ai) : Action(ai, "check stealth") {}
        virtual bool isPossible() { return true; }
        virtual bool Execute(Event& event) {
            bool hasStealth = ai->HasAura("stealth", bot);
            if (hasStealth)
            {
                if (!ai->HasStrategy("stealthed", BotState::BOT_STATE_COMBAT))
                    ai->ChangeStrategy("+stealthed", BotState::BOT_STATE_COMBAT);
            }
            else if (!hasStealth)
            {               
                if (ai->HasStrategy("stealthed", BotState::BOT_STATE_COMBAT))
                {
                    if (bot->GetLevel() < 15)
                        ai->ChangeStrategy("+dps,-stealthed", BotState::BOT_STATE_COMBAT);
                    else
                    {
                        int tab = AiFactory::GetPlayerSpecTab(bot);
                            if (tab == 0)
                                ai->ChangeStrategy("+assassin,-stealthed", BotState::BOT_STATE_COMBAT);
                            else if (tab == 1)
                                ai->ChangeStrategy("+combat,-stealthed", BotState::BOT_STATE_COMBAT);
                            else
                                ai->ChangeStrategy("+subtlety,-stealthed", BotState::BOT_STATE_COMBAT);
                    }
                }
            }
            return true;
        }
    };

	class CastKickAction : public CastMeleeSpellAction
	{
	public:
		CastKickAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "kick") {}
	};

	class CastFeintAction : public CastBuffSpellAction
	{
	public:
		CastFeintAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "feint") {}
	};

	class CastDismantleAction : public CastSpellAction
	{
	public:
		CastDismantleAction(PlayerbotAI* ai) : CastSpellAction(ai, "dismantle") {}
	};

	class CastDistractAction : public CastSpellAction
	{
	public:
		CastDistractAction(PlayerbotAI* ai) : CastSpellAction(ai, "distract") {}
	};

	class CastVanishAction : public CastBuffSpellAction
	{
	public:
		CastVanishAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "vanish") {}
        virtual bool isUseful()
        {
            // do not use with WSG flag or EYE flag
            return !ai->HasAura(23333, bot) && !ai->HasAura(23335, bot) && !ai->HasAura(34976, bot);
        }
        virtual bool Execute(Event& event)
        {
            if (ai->CastSpell("vanish", bot))
            {
                ai->ChangeStrategy("+stealthed", BotState::BOT_STATE_COMBAT);
                bot->InterruptSpell(CURRENT_MELEE_SPELL);
                return true;
            }
            return false;
        }
	};

    MELEE_DEBUFF_ACTION_R(CastBlindAction, "blind", 10.0f);

	class CastBladeFlurryAction : public CastBuffSpellAction
	{
	public:
		CastBladeFlurryAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "blade flurry") {}
	};

	class CastAdrenalineRushAction : public CastBuffSpellAction
	{
	public:
		CastAdrenalineRushAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "adrenaline rush") {}
	};

	class CastKillingSpreeAction : public CastBuffSpellAction
	{
	public:
		CastKillingSpreeAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "killing spree") {}
	};

    class CastKickOnEnemyHealerAction : public CastSpellOnEnemyHealerAction
    {
    public:
        CastKickOnEnemyHealerAction(PlayerbotAI* ai) : CastSpellOnEnemyHealerAction(ai, "kick")
        {
            range = ATTACK_DISTANCE;
        }

    private:
        string GetReachActionName() override { return "reach melee"; }
    };

    class CastTricksOfTheTradeOnPartyAction : public BuffOnPartyAction 
    {
    public:
        CastTricksOfTheTradeOnPartyAction(PlayerbotAI* ai) : BuffOnPartyAction(ai, "tricks of the trade") {}
    };

    class CastCloakOfShadowsAction : public CastCureSpellAction
    {
    public:
        CastCloakOfShadowsAction(PlayerbotAI* ai) : CastCureSpellAction(ai, "cloak of shadows") {}
    };
}
