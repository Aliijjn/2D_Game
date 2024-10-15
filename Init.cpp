#include "2DGame.hpp"
#include "Inventory.hpp"

bool	allowLargeObject(GameInfo* info, IVec2 pos)
{
	if (pos.x < 2 || pos.y < 2 || pos.x > MAP_SIZE - 3 || pos.y > MAP_SIZE - 3)
		return false;
	return	info->blocks[pos.x - 1 +	   pos.y * MAP_SIZE].blockIndex == Block::GRASS && info->blocks[pos.x +       pos.y * MAP_SIZE].blockIndex == Block::GRASS && info->blocks[pos.x + 1 +       pos.y * MAP_SIZE].blockIndex == Block::GRASS &&
			info->blocks[pos.x - 1 + (pos.y - 1) * MAP_SIZE].blockIndex == Block::GRASS && info->blocks[pos.x + (pos.y - 1) * MAP_SIZE].blockIndex == Block::GRASS && info->blocks[pos.x + 1 + (pos.y - 1) * MAP_SIZE].blockIndex == Block::GRASS &&
			info->blocks[pos.x - 1 + (pos.y - 2) * MAP_SIZE].blockIndex == Block::GRASS && info->blocks[pos.x + (pos.y - 2) * MAP_SIZE].blockIndex == Block::GRASS && info->blocks[pos.x + 1 + (pos.y - 2) * MAP_SIZE].blockIndex == Block::GRASS;
}

uint8_t		generateGrass(uint32_t randomNbr)
{
	using namespace GrassTex;

	if (randomNbr % FLOWER1 == FLOWER0)
		return randomNbr % 100 < 50 ? FLOWER0 : FLOWER1;
	return randomNbr % FLOWER1;
}

void	initBlocks(GameInfo* info)
{
	uint32_t	randomNbr = 0;
	int			i;

	info->blocks = new Block[MAP_SIZE * MAP_SIZE]{};
	for (int y = 0; y < MAP_SIZE; y++)
	{
		for (int x = 0; x < MAP_SIZE; x++)
		{
			i = x + y * MAP_SIZE;
			if (x == BLOCK_X / 2 + 1 || y == BLOCK_Y / 2 + 1 || x == MAP_SIZE - BLOCK_X / 2 - 1 || y == MAP_SIZE - BLOCK_Y / 2 - 1)
			{
				info->blocks[i] = { Block::WALL, 0 };
				continue;
			}
			randomNbr += tRand(6) + 1;
			info->grassIndex[i] = generateGrass(randomNbr);
		}
	}

	int32_t objCount;
	int32_t	x;
	int32_t	y;

	for (int32_t i = 0; i < Block::COUNT; i++)
	{
		if (Statics::block[i].spawnOdds == -1)
			continue;
		objCount = MAP_SIZE * MAP_SIZE / Statics::block[i].spawnOdds;
		for (int32_t count = 0; count < objCount; count++)
		{
			do
			{
				x = tRand(MAP_SIZE);
				y = tRand(MAP_SIZE);
			} while (allowLargeObject(info, { x, y }) == false);
			info->blocks[x + y * MAP_SIZE].blockIndex = i;
			info->blocks[x + y * MAP_SIZE].imageIndex = x % Statics::block[i].imageCount;
		}
	}
}

void	initStaticBlocks(GameInfo* info)
{
	using namespace Statics;
	                             // name   |  count          |         path        |         count | rarity | solid | drop count | drop item | break time | replacement
	block[Block::GRASS]        = { "Grass",     9, initTextures("Sprites/Grass/Grass",         9),    -1,     false,      -1,    0,                -1,      Block::GRASS };
	block[Block::WALL]         = { "Wall",      1, initTextures("Sprites/Wall",                1),    -1,     true,       -1,    0,                -1,      Block::GRASS };
	block[Block::TREE]         = { "Tree",      1, initTextures("Sprites/Objects/Tree",        1),    400,    true,        5,    Item::WOOD,       15000,   Block::GRASS };
	block[Block::STONE]        = { "Stone Ore", 2, initTextures("Sprites/Objects/StoneOre",    2),    700,    true,        4,    Item::STONE,      20000,   Block::GRASS };
	block[Block::IRON]         = { "Iron Ore",  2, initTextures("Sprites/Objects/IronOre",     2),    1000,   true,        3,    Item::RAW_IRON,   30000,   Block::GRASS };
	block[Block::WOOD_SINGLE]  = { "Branch",    2, initTextures("Sprites/Objects/WoodSingle",  2),    600,    false,       1,    Item::WOOD,       -1,      Block::GRASS };
	block[Block::STONE_SINGLE] = { "Pebble",    3, initTextures("Sprites/Objects/StoneSingle", 3),    600,    false,       1,    Item::STONE,      -1,      Block::GRASS };
	block[Block::CAMPFIRE]     = { "Campfire",  4, initTextures("Sprites/Objects/Campfire",    4),    -1,     false,      -1,    0,                2000,    Block::GRASS };
}

void	initStaticItems(GameInfo* info)
{
	using namespace Statics;
																						       // single | tool | placeable   |   smelt
	item[Item::WOOD]          = { "Wood",          *initTextures("Sprites/Items/Wood",          1), false, false, -1,              -1 };
	item[Item::STONE]         = { "Stone",         *initTextures("Sprites/Items/Stone",         1), false, false, -1,              -1 };
	item[Item::RAW_IRON]      = { "Raw Iron",      *initTextures("Sprites/Items/IronOre",       1), false, false, -1,              Item::IRON_BAR };
	item[Item::IRON_BAR]      = { "Iron",          *initTextures("Sprites/Items/Iron",          1), false, false, -1,              -1 };
	item[Item::STONE_PICKAXE] = { "Stone Pickaxe", *initTextures("Sprites/Tools/StonePickaxe",  1), true,  true , -1,              -1 };
	item[Item::STONE_AXE]     = { "Stone Axe",     *initTextures("Sprites/Tools/StoneAxe",      1), true,  true , -1,              -1 };
	item[Item::IRON_PICKAXE]  = { "Iron Pickaxe",  *initTextures("Sprites/Tools/IronPickaxe",   1), true,  true , -1,              -1 };
	item[Item::IRON_AXE]      = { "Iron Axe",      *initTextures("Sprites/Tools/IronAxe",       1), true,  true , -1,              -1 };
	item[Item::CAMPFIRE]      = { "Campfire",      *initTextures("Sprites/Placeables/Campfire", 1), true,  false, Block::CAMPFIRE, -1 };
}

void	initCrafts(GameInfo* info)
{
	using namespace Statics;

	recipe[CraftingRecipe::STONE_PICKAXE] = { { Item::WOOD, Item::STONE    }, { 4, 2 }, {}, Item::STONE_PICKAXE, {} };
	recipe[CraftingRecipe::STONE_AXE]     = { { Item::WOOD, Item::STONE    }, { 2, 4 }, {}, Item::STONE_AXE,     {} };
	recipe[CraftingRecipe::CAMPFIRE]      = { { Item::STONE                }, { 5 },    {}, Item::CAMPFIRE,      {} };
	recipe[CraftingRecipe::IRON_PICKAXE]  = { { Item::WOOD, Item::IRON_BAR }, { 3, 5 }, {}, Item::IRON_PICKAXE,  {} };
	recipe[CraftingRecipe::IRON_AXE]      = { { Item::WOOD, Item::IRON_BAR }, { 5, 3 }, {}, Item::IRON_AXE,      {} };

	initCraftingSlots(recipe);
}

void	initPlayer(GameInfo* info)
{
	//init inventory
	initStaticItems(info);
	initCrafts(info);
	initSlot(Player::inventory);

	//init player attributes
	Player::pos = { MAP_SIZE / 2, MAP_SIZE / 2 };
	Player::facingRight = true;
	Player::heldItemOrigin = -1;
	Player::image = initTextures("Sprites/Player", 1);
	Player::str.content = std::string("");
}

GameInfo	init()
{
	GameInfo	info{};

	info.screen = { GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
	initWindow(VRAM_X, VRAM_Y, info.screen.x, info.screen.y, "2D Game", "Sprites/Icon.ico");
	info.cursor = { {(float)info.screen.x / 2, (float)info.screen.x / 2}, {0, 0}, false, 0 };

	// init entities
	initPlayer(&info);

	// init playable area
	info.grassIndex = new uint8_t[MAP_SIZE * MAP_SIZE]{};
	initStaticBlocks(&info);
	initBlocks(&info);

	// misc
	info.font = initFont(FONT_SIZE_Y);
	info.crosshair = initTextures("Sprites/Crosshair", 1);
	info.tileUI = initTextures("Sprites/UI/Tile", 2, 16);
	info.overlay = Overlay::NONE;
	return info;
}