#pragma once
#include "../ServerFacade.h"
#include "../ServerFacade.h"
#include "values/ItemUsageValue.h"

char * strstri (const char* str1, const char* str2);

namespace ai
{
    class IterateItemsVisitor
    {
    public:
        IterateItemsVisitor() {}

        virtual bool Visit(Item* item) = 0;
    };

    class FindItemVisitor : public IterateItemsVisitor {
    public:
        FindItemVisitor() : IterateItemsVisitor(), result(0) {}

        virtual bool Visit(Item* item)
        {
            if (!Accept(item->GetProto()))
                return true;

            result.push_back(item);
            return true;
        }

        list<Item*>& GetResult() { return result; }

    protected:
        virtual bool Accept(const ItemPrototype* proto) = 0;

    private:
        list<Item*> result;
    };

    enum IterateItemsMask
    {
        ITERATE_ITEMS_IN_BAGS = 1,
        ITERATE_ITEMS_IN_EQUIP = 2,
        ITERATE_ITEMS_IN_BANK = 4,
        ITERATE_ALL_ITEMS = 255
    };

    class FindUsableItemVisitor : public FindItemVisitor {
    public:
        FindUsableItemVisitor(Player* bot) : FindItemVisitor()
        {
            this->bot = bot;
        }

        virtual bool Visit(Item* item)
        {
            if (bot->CanUseItem(item->GetProto()) == EQUIP_ERR_OK)
                return FindItemVisitor::Visit(item);

            return true;
        }

    private:
        Player* bot;
    };


    class FindItemsByQualityVisitor : public IterateItemsVisitor
    {
    public:
        FindItemsByQualityVisitor(uint32 quality, int count) : IterateItemsVisitor()
        {
            this->quality = quality;
            this->count = count;
        }

        virtual bool Visit(Item* item)
        {
            if (item->GetProto()->Quality != quality)
                return true;

            if (result.size() >= (size_t)count)
                return false;

            result.push_back(item);
            return true;
        }

        list<Item*>& GetResult()
        {
            return result;
        }

    private:
        uint32 quality;
        uint32 count;
        list<Item*> result;
    };

    class FindItemsToTradeByQualityVisitor : public FindItemsByQualityVisitor
    {
    public:
        FindItemsToTradeByQualityVisitor(uint32 quality, int count) : FindItemsByQualityVisitor(quality, count) {}

        virtual bool Visit(Item* item)
        {
            if (item->IsSoulBound())
                return true;

            return FindItemsByQualityVisitor::Visit(item);
        }
    };

    class FindItemsToTradeByClassVisitor : public IterateItemsVisitor
    {
    public:
        FindItemsToTradeByClassVisitor(uint32 itemClass, uint32 itemSubClass, int count)
            : IterateItemsVisitor(), count(count), itemClass(itemClass), itemSubClass(itemSubClass) {}

        virtual bool Visit(Item* item)
        {
            if (item->IsSoulBound())
                return true;

            if (item->GetProto()->Class != itemClass || item->GetProto()->SubClass != itemSubClass)
                return true;

            if (result.size() >= (size_t)count)
                return false;

            result.push_back(item);
            return true;
        }

        list<Item*>& GetResult()
        {
            return result;
        }

    private:
        uint32 itemClass;
        uint32 itemSubClass;
        uint32 count;
        list<Item*> result;
    };

    class QueryItemCountVisitor : public IterateItemsVisitor
    {
    public:
        QueryItemCountVisitor(uint32 itemId)
        {
            count = 0;
            this->itemId = itemId;
        }

        virtual bool Visit(Item* item)
        {
            if (item->GetProto()->ItemId == itemId)
                count += item->GetCount();

            return true;
        }

        uint32 GetCount() { return count; }

    protected:
        uint32 count;
        uint32 itemId;
    };


    class QueryNamedItemCountVisitor : public QueryItemCountVisitor
    {
    public:
        QueryNamedItemCountVisitor(string name) : QueryItemCountVisitor(0)
        {
            this->name = name;
        }

        virtual bool Visit(Item* item)
        {
            const ItemPrototype* proto = item->GetProto();
            if (proto && !proto->Name1 && strstri(proto->Name1, name.c_str()))
                count += item->GetCount();

            return true;
        }

    private:
        string name;
    };

    class FindNamedItemVisitor : public FindItemVisitor {
    public:
        FindNamedItemVisitor(Player* bot, string name) : FindItemVisitor()
        {
            this->name = name;
        }

        virtual bool Accept(const ItemPrototype* proto)
        {
            return proto && proto->Name1 && strstri(proto->Name1, name.c_str());
        }

    private:
        string name;
    };

    class FindItemByIdVisitor : public FindItemVisitor {
    public:
        FindItemByIdVisitor(uint32 id) : FindItemVisitor()
        {
            this->id = id;
        }

        virtual bool Accept(const ItemPrototype* proto)
        {
            return proto->ItemId == id;
        }

    private:
        uint32 id;
    };

    class FindItemByIdsVisitor : public FindItemVisitor {
    public:
        FindItemByIdsVisitor(ItemIds ids) : FindItemVisitor()
        {
            this->ids = ids;
        }

        virtual bool Accept(const ItemPrototype* proto)
        {
            return ids.find(proto->ItemId) != ids.end();
        }

    private:
        ItemIds ids;
    };

    class ListItemsVisitor : public IterateItemsVisitor
    {
    public:
        ListItemsVisitor() : IterateItemsVisitor() {}

        map<uint32, int> items;
        map<uint32, bool> soulbound;

        virtual bool Visit(Item* item)
        {
            uint32 id = item->GetProto()->ItemId;

            if (items.find(id) == items.end())
                items[id] = 0;

            items[id] += item->GetCount();
            soulbound[id] = item->IsSoulBound();
            return true;
        }
    };

    class ItemCountByQuality : public IterateItemsVisitor
    {
    public:
        ItemCountByQuality() : IterateItemsVisitor()
        {
            for (uint32 i = 0; i < MAX_ITEM_QUALITY; ++i)
                count[i] = 0;
        }

        virtual bool Visit(Item* item)
        {
            count[item->GetProto()->Quality]++;
            return true;
        }

    public:
        map<uint32, int> count;
    };


    class FindPotionVisitor : public FindUsableItemVisitor
    {
    public:
        FindPotionVisitor(Player* bot, uint32 effectId) : FindUsableItemVisitor(bot), effectId(effectId) {}

        virtual bool Accept(const ItemPrototype* proto)
        {
            if (proto->Class == ITEM_CLASS_CONSUMABLE && (proto->SubClass == ITEM_SUBCLASS_POTION || proto->SubClass == ITEM_SUBCLASS_FLASK))
            {
                for (int j = 0; j < MAX_ITEM_PROTO_SPELLS; j++)
                {
                    const SpellEntry* const spellInfo = sServerFacade.LookupSpellInfo(proto->Spells[j].SpellId);
                    if (!spellInfo)
                        return false;

                    for (int i = 0 ; i < 3; i++)
                    {
                        if (spellInfo->Effect[i] == effectId)
                            return true;
                    }
                }
            }
            return false;
        }

    private:
        uint32 effectId;
    };

    class FindFoodVisitor : public FindUsableItemVisitor
    {
    public:
        FindFoodVisitor(Player* bot, uint32 spellCategory, bool conjured = false) : FindUsableItemVisitor(bot)
        {
            this->spellCategory = spellCategory;
            this->conjured = conjured;
        }

        virtual bool Accept(const ItemPrototype* proto)
        {
            return proto->Class == ITEM_CLASS_CONSUMABLE &&
                (proto->SubClass == ITEM_SUBCLASS_CONSUMABLE || proto->SubClass == ITEM_SUBCLASS_FOOD) &&
                proto->Spells[0].SpellCategory == spellCategory &&
                (!conjured || proto->IsConjuredConsumable());
        }

    private:
        uint32 spellCategory;
        bool conjured;
    };

    class FindMountVisitor : public FindUsableItemVisitor
    {
    public:
        FindMountVisitor(Player* bot) : FindUsableItemVisitor(bot) {}

        virtual bool Accept(const ItemPrototype* proto)
        {
            for (int j = 0; j < MAX_ITEM_PROTO_SPELLS; j++)
            {
                const SpellEntry* const spellInfo = sServerFacade.LookupSpellInfo(proto->Spells[j].SpellId);
                if (!spellInfo)
                    return false;

                for (int i = 0 ; i < 3; i++)
                {
                    if (spellInfo->EffectApplyAuraName[i] == SPELL_AURA_MOUNTED)
                        return true;
                }
            }

            return false;
        }

    private:
        uint32 effectId;
    };

    class FindPetVisitor : public FindUsableItemVisitor
    {
    public:
        FindPetVisitor(Player* bot) : FindUsableItemVisitor(bot) {}

        virtual bool Accept(const ItemPrototype* proto)
        {
            if (proto->Class == ITEM_CLASS_MISC)
            {
                for (int j = 0; j < MAX_ITEM_PROTO_SPELLS; j++)
                {
                    const SpellEntry* const spellInfo = sServerFacade.LookupSpellInfo(proto->Spells[j].SpellId);
                    if (!spellInfo)
                        return false;

                    for (int i = 0 ; i < 3; i++)
                    {
#ifdef MANGOSBOT_ZERO
                        if (spellInfo->Effect[i] == SPELL_EFFECT_SUMMON_CRITTER)
#else
#ifdef MANGOSBOT_ONE
						if (spellInfo->Effect[i] == SPELL_EFFECT_97)
#else
						if (spellInfo->Effect[i] == SPELL_EFFECT_SUMMON_PET)
#endif
#endif
                            return true;
                    }
                }
            }
            return false;
        }

    };

    class FindAmmoVisitor : public FindUsableItemVisitor
    {
    public:
        FindAmmoVisitor(Player* bot, uint32 weaponType) : FindUsableItemVisitor(bot)
        {
            this->weaponType = weaponType;
        }

        virtual bool Accept(const ItemPrototype* proto)
        {
            if (proto->Class == ITEM_CLASS_PROJECTILE)
            {
                uint32 subClass = 0;
                switch (weaponType)
                {
                case ITEM_SUBCLASS_WEAPON_GUN:
                    subClass = ITEM_SUBCLASS_BULLET;
                    break;
                case ITEM_SUBCLASS_WEAPON_BOW:
                case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                    subClass = ITEM_SUBCLASS_ARROW;
                    break;
                }

                if (!subClass)
                    return false;

                if (proto->SubClass == subClass)
                    return true;
            }
            return false;
        }
    private:
        uint32 weaponType;
    };

    class FindQuestItemVisitor : public FindUsableItemVisitor
    {
    public:
        FindQuestItemVisitor(Player* bot) : FindUsableItemVisitor(bot) {}

        virtual bool Accept(const ItemPrototype* proto)
        {
            if (proto->Class == ITEM_CLASS_QUEST)
            {
                return true;
            }
            return false;
        }
    };

    class FindRecipeVisitor : public FindUsableItemVisitor
    {
    public:
        FindRecipeVisitor(Player* bot, SkillType skill = SKILL_NONE) : FindUsableItemVisitor(bot), skill(skill) {};

        virtual bool Accept(const ItemPrototype* proto)
        {
            if (proto->Class == ITEM_CLASS_RECIPE)
            {
                if (skill == SKILL_NONE)
                    return true;

                switch (proto->SubClass)
                {
                case ITEM_SUBCLASS_LEATHERWORKING_PATTERN:
                    return skill == SKILL_LEATHERWORKING;
                case ITEM_SUBCLASS_TAILORING_PATTERN:
                    return skill == SKILL_TAILORING;
                case ITEM_SUBCLASS_ENGINEERING_SCHEMATIC:
                    return skill == SKILL_ENGINEERING;
                case ITEM_SUBCLASS_BLACKSMITHING:
                    return skill == SKILL_BLACKSMITHING;
                case ITEM_SUBCLASS_COOKING_RECIPE:
                    return skill == SKILL_COOKING;
                case ITEM_SUBCLASS_ALCHEMY_RECIPE:
                    return skill == SKILL_ALCHEMY;
                case ITEM_SUBCLASS_FIRST_AID_MANUAL:
                    return skill == SKILL_FIRST_AID;
                case ITEM_SUBCLASS_ENCHANTING_FORMULA:
                    return skill == SKILL_ENCHANTING;
                case ITEM_SUBCLASS_FISHING_MANUAL:
                    return skill == SKILL_FISHING;
                }
            }
            return false;
        }
    private:
        SkillType skill;
    };

    class FindItemUsageVisitor : public FindUsableItemVisitor
    {
    public:
        FindItemUsageVisitor(Player* bot, ItemUsage usage = ITEM_USAGE_NONE) : FindUsableItemVisitor(bot), usage(usage) { context = bot->GetPlayerbotAI()->GetAiObjectContext();};

        virtual bool Accept(const ItemPrototype* proto)
        {
            if (AI_VALUE2(ItemUsage, "item usage", proto->ItemId) == usage)
                return true;

            return false;
        }
    private:
        AiObjectContext* context;
        ItemUsage usage;
    };
}
