#include "2DGame.hpp"

void	initCraftingSlots(CraftingRecipe* craft)
{
	for (int y = 0; y < CraftingRecipe::COUNT; y++)
	{
		int x = 0;

		for (; x < MAX_INGREDIENTS; x++)
		{
			craft[y].requirements[x] = { 
				{ 20.0f + x * (BLOCK_S + 2), 55.0f + INVENTORY_SIZE / INVENTORY_WIDTH * (BLOCK_S + 2) + y * (BLOCK_S + 2)}, 
				{ BLOCK_S, BLOCK_S },
				{ craft[y].requirementIndex[x], craft[y].requirementCount[x] }
			};
		}
		craft[y].result = {
			{ 20.0f + x * (BLOCK_S + 2), 55.0f + INVENTORY_SIZE / INVENTORY_WIDTH * (BLOCK_S + 2) + y * (BLOCK_S + 2)},
			{ BLOCK_S, BLOCK_S },
			{ craft[y].resultIndex, 1 }
		};
	}
}

void	initSlot(Slot* slots)
{
	for (int i = 0; i < INVENTORY_SIZE; i++)
	{
		slots[i].pos.x = 20.0f + i % INVENTORY_WIDTH * (BLOCK_S + 2);
		slots[i].pos.y = 30.0f + i / INVENTORY_WIDTH * (BLOCK_S + 2) + (i >= INVENTORY_WIDTH ? 4.0f : 0.0f);
	}
}

//--------------------------------- ^^^INIT ^^^ -----------------------------------------

bool	toInventory(GameInfo* info, uint16_t itemIndex, uint16_t count)
{
	std::string itemName = Statics::item[itemIndex].name;

	for (int i = 0; i < INVENTORY_SIZE; i++)
	{
		//check existing stack of items
		Item* item = &Player::inventory[i].item;
		if (item->index == itemIndex)
		{
			if (item->count + count <= STACK_SIZE && Statics::item[item->index].isSingle == false)
			{
				item->count += count;
				setPlayerString("+" + std::to_string(count) + " " + itemName);
				return true;
			}
			else if (Statics::item[item->index].isSingle == false)
			{
				count -= STACK_SIZE - item->count;
				item->count = STACK_SIZE;
			}
		}
	}
	//make new stack of items
	for (int i = 0; i < INVENTORY_SIZE; i++)
	{
		//check existing stack of items
		Item* item = &Player::inventory[i].item;
		if (item->index == Item::EMPTY)
		{
			item->index = itemIndex;
			item->count = count;
			setPlayerString("+" + std::to_string(count) + " " + itemName);
			return true;
		}
	}
	setPlayerString("Inventory full!");
	return false;
}

uint32_t    getItemCount(GameInfo* info, uint16_t itemIndex)
{
	uint32_t    count = 0;

	for (int i = 0; i < INVENTORY_SIZE; i++)
	{
		if (Player::inventory[i].item.index == itemIndex)
			count += Player::inventory[i].item.count;
	}
	return count;
}

void    removeItem(GameInfo* info, uint16_t itemIndex, int32_t count)
{
	for (int i = 0; i < INVENTORY_SIZE; i++)
	{
		Slot* slot = &Player::inventory[i];
		if (slot->item.index == itemIndex)
		{
			if (count - slot->item.count <= 0)
			{
				slot->item.count -= count;
				if (slot->item.count == 0)
					slot->item.index = Item::EMPTY;
				return;
			}
			else
			{
				count -= slot->item.count;
				slot->item.count = 0;
				slot->item.index = Item::EMPTY;
			}
		}
	}
}

void	manageDrag(GameInfo* info)
{
	if (Player::heldItem.index != Item::EMPTY)
		return;
	// draging within inventory
	for (uint32_t i = 0; i < INVENTORY_SIZE; i++)
	{
		Slot*	slot = &Player::inventory[i];

		if (slot->item.index != Item::EMPTY && AABB(slot->pos, slot->size, info->cursor.pos) == true)
		{
			Player::heldItem = slot->item;
			Player::heldItemOrigin = i;
			return;
		}
	}
	// crafting
	for (uint32_t i = 0; i < CraftingRecipe::COUNT; i++)
	{
		CraftingRecipe*	craftOut = &Statics::recipe[i];

		//std::cout << craftOut->result.size.x << "\n";
		if (AABB(craftOut->result.pos, craftOut->result.size, info->cursor.pos) == true)
		{
			for (int j = 0; craftOut->requirementIndex[j] != Item::EMPTY && j < CraftingRecipe::COUNT; j++)
			{
				if (getItemCount(info, craftOut->requirementIndex[j]) < craftOut->requirementCount[j] && CHEATS == false)
				{
					return;
				}
			}
			for (int j = 0; craftOut->requirementIndex[j] != Item::EMPTY && j < CraftingRecipe::COUNT; j++)
			{
				removeItem(info, craftOut->requirementIndex[j], craftOut->requirementCount[j]);
			}
			Player::heldItem.index = craftOut->resultIndex;
			Player::heldItem.count = 1;
			Player::heldItemOrigin = -1;
			return;
		}
	}
	for (auto iter = Entities::fireplace.begin(); iter != Entities::fireplace.end(); )
	{
		if (AABB(mapToCursor(iter->output.pos), iter->output.size, info->cursor.pos) == true)
		{
			Player::heldItem = iter->output.item;
			iter->output.item = { 0, 0 };
			return;
		}
		iter++;
	}
}

void	manageDrop(GameInfo* info)
{
	using namespace Player;

	if (heldItem.index == Item::EMPTY)
		return;

	for (uint32_t i = 0; i < INVENTORY_SIZE; i++)
	{
		Slot* slot = &inventory[i];

		if (AABB(slot->pos, slot->size, info->cursor.pos) == true)
		{
			Item* item = &slot->item;

			if (item->index == heldItem.index 
				&& i != heldItemOrigin 
				&& (int)item->count + heldItem.count <= STACK_SIZE
				&& Statics::item[item->index].isSingle == false)
			{
				item->count += heldItem.count;
				inventory[heldItemOrigin].item = { Item::EMPTY, 0 };
				heldItem = { Item::EMPTY, 0 };
				std::cout << "1\n";
			}
			else if (i != heldItemOrigin && heldItemOrigin != -1)
			{
				inventory[heldItemOrigin].item = *item;
				*item = heldItem;
				heldItem = { Item::EMPTY, 0 };
				std::cout << "2\n";
			}
			else if (item->index == Item::EMPTY)
			{
				*item = heldItem;
				heldItem = { Item::EMPTY, 0 };
				std::cout << "3\n";
			}
			heldItemOrigin = -1;
			return;
		}
		for (auto iter = Entities::fireplace.begin(); iter != Entities::fireplace.end(); )
		{
			if (AABB(mapToCursor(iter->input.pos), iter->input.size, info->cursor.pos) == true)
			{
				iter->input.item = heldItem;
				heldItem = {0, 0};
				inventory[heldItemOrigin].item.index = Item::EMPTY;
				inventory[heldItemOrigin].item.count = 0;
				heldItemOrigin = -1;
				return;
			}
			iter++;
		}
	}
	if (info->cursor.pos.x > (inventory[INVENTORY_SIZE - 1].pos.x + BLOCK_S + 4))
	{
		addDroppedItem(heldItem, pos, subtractVec2(cursorToMap(info->cursor.pos), Player::pos), 1);
		heldItem.count = 0;
		heldItem.index = Item::EMPTY;
		inventory[heldItemOrigin].item.index = Item::EMPTY;
		inventory[heldItemOrigin].item.count = 0;
		heldItemOrigin = -1;
		return;
	}
}
