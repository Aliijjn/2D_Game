#include "2DGame.hpp"
#include "Render.hpp"
#include "Inventory.hpp"

uint32_t	calcBreakSpeed(GameInfo* info, uint16_t breakableType)
{
	uint16_t	currentItem = Player::inventory[Player::hotbarIndex].item.index;
	if (CHEATS == true)
		return 1000;
	if (currentItem == Item::STONE_PICKAXE && (breakableType == Block::STONE || breakableType == Block::IRON))
		return 7;
	if (currentItem == Item::IRON_PICKAXE && (breakableType == Block::STONE || breakableType == Block::IRON))
		return 35;
	if (currentItem == Item::STONE_AXE && breakableType == Block::TREE)
		return 5;
	if (currentItem == Item::IRON_AXE && breakableType == Block::TREE)
		return 25;
	return 1;
}

void	manageCampfire(GameInfo* info)
{
	uint32_t		x = info->cursor.blockIndex.x;
	uint32_t		y = info->cursor.blockIndex.y;
	Block*			targetBlock = &info->blocks[x + y * MAP_SIZE];
	StaticBlock*	targetSBlock = &Statics::block[targetBlock->blockIndex];
	Item*			heldItem = &Player::inventory[Player::hotbarIndex].item;
	StaticItem*		heldSItem = &Statics::item[Player::inventory[Player::hotbarIndex].item.index];

	static DWORD	previousTime;

	if (heldItem->index == Item::WOOD)
	{
		Fireplace*	fireplace = (Fireplace*)targetBlock->entity;
		if (fireplace->logCount <= Fireplace::LOG_MAX)
		{
			drawBottomString(info, "Press [MOUSE2] light fire");
		}
		if (previousTime + 200 > timeGetTime() || info->cursor.rightClick == false)
		{
			return;
		}
		if (fireplace->logCount > Fireplace::LOG_MAX)
		{
			setPlayerString("Campfire full");
		}
		else
		{
			fireplace->logCount += Fireplace::LOG_VAL;
			heldItem->count--;
			if (heldItem->count == 0)
				heldItem->index = Item::EMPTY;
		}
		previousTime = timeGetTime();
	}
}

void	pickUpItems(GameInfo* info)
{
	Block*			standingOn = &info->blocks[(int)Player::pos.x + 1 + ((int)Player::pos.y + 1) * MAP_SIZE];
	StaticBlock*	standingOnStatic = &Statics::block[standingOn->blockIndex];

	if (standingOnStatic->pickupCount != -1)
	{
		if (toInventory(info, standingOnStatic->pickupIndex, standingOnStatic->pickupCount) == true)
			standingOn->blockIndex = standingOnStatic->replacementIndex;
	}
	for (auto iter = Entities::itemEntity.begin(); iter != Entities::itemEntity.end();)
	{
		if (AABB({ Player::pos.x - 0.5f, Player::pos.y - 0.5f }, { 1, 1 }, iter->pos) == true && iter->timeActive > 100)
		{
			if (toInventory(info, iter->item.index, iter->item.count) == true);
				iter = Entities::itemEntity.erase(iter);
		}
		else
		{
			iter++;
		}
	}
}

void	spitOutItems(uint16_t itemIndex, uint16_t count, Vec2 pos)
{
	for (int i = 0; i < count; i++)
	{
		addDroppedItem({ itemIndex, 1 }, pos, {(float)tRand(40) - 20, (float)tRand(40) - 20}, 0.35f);
	}
}

void	breakItem(GameInfo* info)
{
	uint32_t		x = info->cursor.blockIndex.x;
	uint32_t		y = info->cursor.blockIndex.y;
	Block*			targetBlock = &info->blocks[x + y * MAP_SIZE];
	StaticBlock*	targetSBlock = &Statics::block[targetBlock->blockIndex];
	int32_t		    remainingTime = targetSBlock->breakTimeMS / calcBreakSpeed(info, targetBlock->blockIndex) - (timeGetTime() - info->cursor.activeTime);

	if (targetSBlock->breakTimeMS != -1 && info->cursor.leftClick == true)
	{
		if (remainingTime <= 0 && targetSBlock->pickupCount != -1)
		{
			spitOutItems( targetSBlock->pickupIndex, targetSBlock->pickupCount, toVec2({(int)x, (int)y}));
			targetBlock->blockIndex = targetSBlock->replacementIndex;
			targetBlock->imageIndex = tRand(Statics::block[targetBlock->blockIndex].imageCount);
		}
		else
			drawCursorString(info, std::to_string(remainingTime / 1000) + "." + std::to_string(remainingTime / 100 % 10) + "s", true);
	}
	else if (targetSBlock->breakTimeMS != -1 || targetSBlock->pickupCount != -1)
	{
		drawCursorString(info, targetSBlock->name, true);
		if (targetSBlock->breakTimeMS != -1)
			drawBottomString(info, "Hold [MOUSE1] to break");
	}
}

void	gameLogic(GameInfo* info)
{
	uint32_t		x = info->cursor.blockIndex.x;
	uint32_t		y = info->cursor.blockIndex.y;
	Block*			targetBlock = &info->blocks[x + y * MAP_SIZE];
	StaticBlock*	targetSBlock = &Statics::block[targetBlock->blockIndex];
	Item*			heldItem = &Player::inventory[Player::hotbarIndex].item;
	StaticItem*		heldSItem = &Statics::item[Player::inventory[Player::hotbarIndex].item.index];

	pickUpItems(info);
	if (heldSItem->blockIndex != -1)
	{
		drawBottomString(info, "Press [MOUSE2] to place");
		if (info->cursor.rightClick == true)
		{
			IVec2	mapPos = toIVec2(cursorToMap(info->cursor.pos));
			if (info->blocks[mapPos.x + mapPos.y * MAP_SIZE].blockIndex != Block::GRASS)
				setPlayerString("Invalid position");
			else
			{
				Fireplace	fireplace{};

				fireplace.input.pos  = toVec2({ mapPos.x - 1, mapPos.y + 1 });
				fireplace.output.pos = toVec2({ mapPos.x + 1, mapPos.y + 1 });
				
				Entities::fireplace.push_front(fireplace);
				info->blocks[mapPos.x + mapPos.y * MAP_SIZE].blockIndex = heldSItem->blockIndex;
				heldItem->index = Item::EMPTY;
				heldItem->count = 0;
				targetBlock->blockIndex = Block::CAMPFIRE;
				targetBlock->entity = &Entities::fireplace.front();
			}
		}
	}

	switch (targetBlock->blockIndex)
	{
	case Block::CAMPFIRE:
		manageCampfire(info);
		return;
	}

	breakItem(info);
	if (Player::str.content != "" && Player::str.initTime + Player::str.maxTime < timeGetTime())
	{
		Player::str.content = "";
	}
	if (info->overlay == Overlay::INVENTORY)
	{
		if (info->cursor.leftClick == true)
			manageDrag(info);
		else
			manageDrop(info);
	}
}

void	tick(GameInfo* info, uint64_t& ticks)
{
	uint64_t	tickDelta = tickGet() - ticks;
	for (int i = 0; i < tickDelta; i++)
	{
		for (Fireplace& fireplace : Entities::fireplace)
		{
			if (fireplace.logCount != 0)
				fireplace.logCount--;
		}
		for (auto iter = Entities::itemEntity.begin(); iter != Entities::itemEntity.end();)
		{
			//std::cout << "Item dir: " << iter->dir.x << " " << iter->dir.y << " \n";
			iter->pos.x += iter->dir.x * DroppedItem::MULTIPLIER;
			iter->pos.y += iter->dir.y * DroppedItem::MULTIPLIER;
			iter->dir.x *= DroppedItem::FRICTION;
			iter->dir.y *= DroppedItem::FRICTION;
			iter->timeActive++;
			iter++;
			//std::cout << "Item dir: " << iter->dir.x << " " << iter->dir.y << " \n";
		}
		for (auto iter = Entities::fireplace.begin(); iter != Entities::fireplace.end(); )
		{
			if (Statics::item[iter->input.item.index].smeltIndex != -1 && iter->logCount > 0 && iter->output.item.count < STACK_SIZE
				&& (iter->output.item.index == Item::EMPTY || iter->output.item.index == Statics::item[iter->input.item.index].smeltIndex))
			{
				iter->tickCount++;
				if (iter->tickCount >= iter->SMELT_TIME)
				{
					iter->output.item.count++;
					iter->output.item.index = Statics::item[iter->input.item.index].smeltIndex;
					iter->input.item.count--;
					if (iter->input.item.count == 0)
						iter->input.item.index = Item::EMPTY;
					iter->tickCount = 0;
				}
			}
			iter++;
		}
	}
	ticks += tickDelta;
}

int main()
{
	GameInfo	info = init();
	uint64_t	tickCount = tickGet();

	while (true)
	{
		getMovement(&info);
		render(&info);
		gameLogic(&info);
		tick(&info, tickCount);
		toScreen();
		drawBottomString(&info, "", true);
		//countFPS();
	}
	return 0;
}