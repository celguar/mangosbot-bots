#pragma once
#include "GenericActions.h"
#include "../../ServerFacade.h"
#include "../../RandomItemMgr.h"

namespace ai
{
    //This class bypasses the requirement for a bot to have a key item in their inventory when opening a lock.
    class BotUseItemSpell : public Spell 
    {
    public:
        BotUseItemSpell(WorldObject* caster, SpellEntry const* info, uint32 triggeredFlags, ObjectGuid originalCasterGUID = ObjectGuid(), SpellEntry const* triggeredBy = nullptr, bool itemCheats = false) : Spell(caster, info, triggeredFlags, originalCasterGUID, triggeredBy), itemCheats(itemCheats) {};
        SpellCastResult ForceSpellStart(SpellCastTargets const* targets, Aura* triggeredByAura = nullptr);
        bool OpenLockCheck();

    private:
        bool itemCheats;
    };

    class UseItemAction : public ChatCommandAction 
    {
    public:
        UseItemAction(PlayerbotAI* ai, string name = "use", bool selfOnly = false, uint32 duration = sPlayerbotAIConfig.reactDelay) : ChatCommandAction(ai, name, duration), selfOnly(selfOnly) {}

    public:
        virtual bool isPossible() override;

        // Used when this action is executed as a reaction
        virtual bool ShouldReactionInterruptCast() const override { return true; }

    protected:
        virtual bool Execute(Event& event) override;
        bool UseItemAuto(Item* item);
        bool UseItemOnGameObject(Item* item, ObjectGuid go);
        bool UseItemOnItem(Item* item, Item* itemTarget);
        bool UseItemOnTarget(Item* item, Unit* target);
        bool UseItem(Item* item, ObjectGuid go, Item* itemTarget, Unit* unitTarget = nullptr);
        bool UseGameObject(ObjectGuid guid);
        bool SocketItem(Item * item, Item * gem, bool replace = false);
        void TellConsumableUse(Item* item, string action, float percent);

    private:
        bool selfOnly;
    };

    class UseItemIdAction : public UseItemAction, public Qualified
    {
    public:
        UseItemIdAction(PlayerbotAI* ai, string name = "use id", bool selfOnly = false, uint32 duration = sPlayerbotAIConfig.reactDelay) : UseItemAction(ai, name, selfOnly, duration), Qualified() {}
        virtual bool isPossible() override;

    protected:
        virtual bool Execute(Event& event) override;
        virtual uint32 GetItemId() { return getQualifier().empty() ? 0 : getMultiQualifierInt(getQualifier(),0, ","); }
        virtual Unit* GetTarget() override { return nullptr; }
        bool HasSpellCooldown(const uint32 itemId);
        bool CastItemSpell(uint32 itemId, Unit* target, GameObject* goTarget);
    };

    class UseTargetedItemIdAction : public UseItemIdAction
    {
    public:
        UseTargetedItemIdAction(PlayerbotAI* ai, string name, bool selfOnly = false, uint32 duration = sPlayerbotAIConfig.reactDelay) : UseItemIdAction(ai, name, selfOnly, duration) {}
        virtual Unit* GetTarget() override { return Action::GetTarget(); }
        virtual uint32 GetItemId() override { return  0; }
    };

    class UseSpellItemAction : public UseItemAction 
    {
    public:
        UseSpellItemAction(PlayerbotAI* ai, string name, bool selfOnly = false) : UseItemAction(ai, name, selfOnly) {}
        virtual bool isUseful() override;
    };

    class UsePotionAction : public UseItemIdAction
    {
    public:
        UsePotionAction(PlayerbotAI* ai, string name, SpellEffects effect) : UseItemIdAction(ai, name), effect(effect) {}

        bool isUseful() override { return AI_VALUE2(bool, "combat", "self target"); }

        virtual uint32 GetItemId() override
        {
            list<Item*> items = AI_VALUE2(list<Item*>, "inventory items", getName());
            if (items.empty())
            {
                return sRandomItemMgr.GetRandomPotion(bot->GetLevel(), effect);
            }

            return items.front()->GetProto()->ItemId;
        }

    private:
        SpellEffects effect;
    };

    class UseHealingPotionAction : public UsePotionAction
    {
    public:
        UseHealingPotionAction(PlayerbotAI* ai) : UsePotionAction(ai, "healing potion", SPELL_EFFECT_HEAL) {}
    };

    class UseManaPotionAction : public UsePotionAction
    {
    public:
        UseManaPotionAction(PlayerbotAI* ai) : UsePotionAction(ai, "mana potion", SPELL_EFFECT_ENERGIZE) {}
    };

    class UseHearthStoneAction : public UseItemAction
    {
    public:
        UseHearthStoneAction(PlayerbotAI* ai) : UseItemAction(ai, "hearthstone", true, 10000U) {}

        virtual bool Execute(Event& event) override;

        bool isUseful() override;
    
        // Used when this action is executed as a reaction
        bool ShouldReactionInterruptMovement() const override { return true; }
    };

    class UseRandomRecipeAction : public UseItemAction
    {
    public:
        UseRandomRecipeAction(PlayerbotAI* ai) : UseItemAction(ai, "random recipe", true, 3000U) {}

        virtual bool isUseful() override;
        virtual bool isPossible() override {return AI_VALUE2(uint32,"item count", "recipe") > 0; }
      
        virtual bool Execute(Event& event) override;

        // Used when this action is executed as a reaction
        bool ShouldReactionInterruptMovement() const override { return true; }
    };

    class UseRandomQuestItemAction : public UseItemAction
    {
    public:
        UseRandomQuestItemAction(PlayerbotAI* ai) : UseItemAction(ai, "use random quest item", true) {}

        virtual bool isUseful() override;
        virtual bool isPossible() override { return AI_VALUE2(uint32, "item count", "quest") > 0;}

        virtual bool Execute(Event& event) override;

        // Used when this action is executed as a reaction
        bool ShouldReactionInterruptMovement() const override { return true; }
    };

    // goblin sappers
    class UseGoblinSapperChargeAction : public UseItemIdAction
    {
    public:
        UseGoblinSapperChargeAction(PlayerbotAI* ai) : UseItemIdAction(ai, "goblin sapper") {}
        virtual bool isUseful() override { return bot->GetSkillValue(202) >= 205 && bot->GetHealth() > 1000; }

        virtual uint32 GetItemId() override
        { 
#ifndef MANGOSBOT_ZERO
            if (bot->InArena())
                return false;
#endif
            return (bot->GetLevel() >= 68) ? 23827 : 10646; 
        }
    };

    // oil of immolation
    class UseOilOfImmolationAction : public UseItemIdAction
    {
    public:
        UseOilOfImmolationAction(PlayerbotAI* ai) : UseItemIdAction(ai, "oil of immolation") {}
        virtual bool isUseful() override
        {
#ifndef MANGOSBOT_ZERO
            if (bot->InArena())
                return false;
#endif
            return bot->GetLevel() >= 31 && !ai->HasAura(11350, bot);
        }

        virtual uint32 GetItemId() override { return 8956; }
    };

    class UseBgBannerAction : public UseItemIdAction
    {
    public:
        UseBgBannerAction(PlayerbotAI* ai) : UseItemIdAction(ai, "bg banner") {}

        virtual bool isUseful() override
        {
            if (!bot->InBattleGround() || bot->GetLevel() < 60 || !bot->IsInCombat())
                return false;

            list<ObjectGuid> units = *context->GetValue<list<ObjectGuid> >("nearest npcs no los");
            for (list<ObjectGuid>::iterator i = units.begin(); i != units.end(); i++)
            {
                Unit* unit = ai->GetUnit(*i);
                if (!unit)
                    continue;

                if (bot->GetTeam() == HORDE && unit->GetEntry() == 14466)
                    return false;

                if (bot->GetTeam() == ALLIANCE && unit->GetEntry() == 14465)
                    return false;
            }

            return true;
        }

        virtual uint32 GetItemId() override
        {
            return bot->GetTeam() == ALLIANCE ? 18606 : 18607;
        }

        // Used when this action is executed as a reaction
        bool ShouldReactionInterruptMovement() const override { return true; }
    };

    class UseRocketBootsAction : public UseItemIdAction
    {
    public:
        UseRocketBootsAction(PlayerbotAI* ai) : UseItemIdAction(ai, "rocket boots") {}

        virtual bool isUseful() override
        {
            if (ai->HasAnyAuraOf(bot, "sprint", "speed", "goblin rocket boots", "dash", NULL))
                return false;

            return true;
        }

        virtual uint32 GetItemId() override
        {
            return 7189;
        }
    };

    class UseBandageAction : public UseTargetedItemIdAction
    {
    public:
        UseBandageAction(PlayerbotAI* ai) : UseTargetedItemIdAction(ai, "use bandage", false, 8000U) {}

        virtual bool isUseful() override
        {
            if (bot->HasAura(11196))
                return false;

            if (AI_VALUE(uint8, "my attacker count") > 0 || bot->HasAuraType(SPELL_AURA_PERIODIC_DAMAGE))
                return false;

            if (bot->GetSkillValue(129) < 1)
                return false;

            return true;
        }

        virtual string GetTargetName() override { return "self target"; }

        virtual uint32 GetItemId() override
        {
            if (bot->GetSkillValue(129) >= 225)
                return 14530;
            if (bot->GetSkillValue(129) >= 200)
                return 14529;
            if (bot->GetSkillValue(129) >= 175)
                return 8545;
            if (bot->GetSkillValue(129) >= 150)
                return 8544;
            if (bot->GetSkillValue(129) >= 125)
                return 6451;
            if (bot->GetSkillValue(129) >= 100)
                return 6450;
            if (bot->GetSkillValue(129) >= 75)
                return 3531;
            if (bot->GetSkillValue(129) >= 50)
                return 3530;
            if (bot->GetSkillValue(129) >= 20)
                return 2581;
            return 1251;
        }

        // Used when this action is executed as a reaction
        bool ShouldReactionInterruptMovement() const override { return true; }
    };

    class UseAdamantiteGrenadeAction : public UseTargetedItemIdAction
    {
    public:
        UseAdamantiteGrenadeAction(PlayerbotAI* ai) : UseTargetedItemIdAction(ai, "adamantite grenade") {}

        virtual string GetTargetName() override { return "current target"; }

        virtual bool isUseful() override
        {
#ifndef MANGOSBOT_ZERO
            if (bot->InArena())
                return false;
#endif

            Unit* target = GetTarget();
            if (!target)
                return false;

            if (!target->IsNonMeleeSpellCasted(false) && target->IsStopped())
                return false;

            if (bot->GetSkillValue(202) < 175)
                return false;

            return bot->GetLevel() >= 52;
        }

        virtual uint32 GetItemId() 
        { 
            if (bot->GetSkillValue(202) >= 325)
                return 23737;
            if (bot->GetSkillValue(202) >= 260)
                return 15993;
            return 4390; 
        }
    };

    class UseDarkRuneAction : public UseItemIdAction
    {
    public:
        UseDarkRuneAction(PlayerbotAI* ai) : UseItemIdAction(ai, "dark rune") {}

        virtual bool isUseful() override
        {
#ifndef MANGOSBOT_ZERO
            if (bot->InArena())
                return false;
#endif

            if (bot->getClass() == CLASS_MAGE) // mage should use mana gem, shares cd with dark rune
                return false;

            return bot->GetHealth() > 1000;
        }

        virtual uint32 GetItemId() override { return 20520; }
    };

    class UseFireProtectionPotionAction : public UseItemIdAction
    {
    public:
        UseFireProtectionPotionAction(PlayerbotAI* ai) : UseItemIdAction(ai, "fire protection potion") {}

        virtual bool isUseful() override
        {
            return !bot->HasAura(17543) && (bot->GetLevel() >= 48);
        }

        virtual uint32 GetItemId() override { return 13457; }
    };

    class UseFreeActionPotionAction : public UseItemIdAction
    {
    public:
        UseFreeActionPotionAction(PlayerbotAI* ai) : UseItemIdAction(ai, "free action potion") {}

        virtual bool isUseful() override
        {
            if (bot->GetLevel() < 20 || bot->HasAura(6615))
                return false;

            Unit* target = AI_VALUE(Unit*, "current target");
            if (!target || !target->IsPlayer())
                return false;

            if (!bot->InBattleGround() && urand(0, 1))
                return false;

            return true;
        }

        virtual uint32 GetItemId() override { return 5634; }
    };
}
