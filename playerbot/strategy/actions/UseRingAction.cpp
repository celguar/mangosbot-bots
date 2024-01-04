#include "botpch.h"
#include "../../playerbot.h"
#include "UseRingAction.h"
#include "Item.h"
#include "ItemPrototype.h"
#include "Player.h"

using namespace ai;

bool UseRingAction::Execute(Event& event)
{
	auto rings = AI_VALUE(list<Item*>, "rings on use");

	if (rings.empty())
		return false;

	for (Item * item : rings)
	{
		ItemPrototype const* proto = item->GetProto();

		if (proto->InventoryType == INVTYPE_FINGER && item->IsEquipped())
		{
			if (bot->CanUseItem(item) == EQUIP_ERR_OK && !item->IsInTrade())
			{
				return UseItemAuto(GetMaster(), item);
			}
		}
	}

    return false;
}

bool UseRingAction::isPossible()
{
	return AI_VALUE(list<Item*>, "rings on use").size() > 0;
}
