#ifndef GAME2D_HPP
#define GAME2D_HPP

#include <Windows.h>
#include <stdlib.h>
#include <time.h>
#include <mmsystem.h>
#include <gl/GL.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>

//player logic
#define MV_SPEED 0.0055f
#define MV_DIAGONAL (MV_SPEED * 0.7f)
#define MV_FRICTION 0.90f

//graphical
#define BLOCK_S 16
#define BLOCK_X 32
#define BLOCK_Y 18
#define VRAM_X (BLOCK_S * BLOCK_X)
#define VRAM_Y (BLOCK_S * BLOCK_Y)
#define VSYNC true

#define FONT_RANGE 128
#define FONT_SIZE_Y 12
#define MESSAGE_UPTIME 2000

//game logic
#define MAX_BULLET 50
#define MAP_SIZE 1000
#define BREAK_SPEED 5000

#define INVENTORY_WIDTH 6
#define INVENTORY_SIZE (INVENTORY_WIDTH * 4)
#define STACK_SIZE 10
#define MAX_INGREDIENTS 3

#define CHEATS true

namespace Overlay
{
    enum
    {
        NONE,
        INVENTORY,
        CRAFTING,
        MAX
    };
}

namespace GrassTex
{
    enum
    {
        GRASS0,
        GRASS1,
        GRASS2,
        GRASS3,
        GRASS4,
        GRASS5,
        GRASS6,
        FLOWER0,
        FLOWER1
    };
}

struct Pixel
{
	uint8_t		r, g, b, a;
};

struct Input
{
    char    w, a, s, d, e;
    int8_t numPressed;
    int32_t mwDir;
};

struct Vec2
{
    float x, y;
};

struct IVec2
{
    int32_t x, y;
};

struct Cursor
{
    Vec2    pos;
    IVec2   blockIndex;
    bool    leftClick;
    bool    rightClick;
    DWORD   activeTime;

};

struct Image
{
    Pixel*      content;
    IVec2       size;
    uint32_t    index;
};

struct GraphicStr
{
    std::string content;
    DWORD       initTime;
    uint32_t    maxTime;
};

struct Item
{
    uint16_t    index;
    uint16_t    count;
    enum
    {
        EMPTY,
        WOOD,
        STONE,
        RAW_IRON,
        IRON_BAR,
        STONE_PICKAXE,
        STONE_AXE,
        IRON_PICKAXE,
        IRON_AXE,
        CAMPFIRE,
        COUNT
    };
};

struct Entity
{
    Vec2    pos, dir;
    bool    facingRight;
    int     timeActive;
    Image*  image;
};

struct ItemEntity : Entity
{
    Item    item;
};

struct Slot
{
    Vec2    pos;
    Vec2    size = { BLOCK_S, BLOCK_S };
    Item    item;
};

struct Block
{
    uint16_t    blockIndex;
    uint16_t    imageIndex;
    void*       entity = NULL;
    enum
    {
        GRASS,
        WALL,
        TREE,
        STONE,
        IRON,
        WOOD_SINGLE,
        STONE_SINGLE,
        CAMPFIRE,
        COUNT
    };
};

struct GameInfo
{
    IVec2                   screen;
    Cursor                  cursor;
    uint8_t*                grassIndex;
    Block*                  blocks;
    Image*                  font;
    Image*                  crosshair;
    Image*                  tileUI;
    uint8_t                 overlay;
};

//-------------------------- vvv globals vvv ---------------------------

struct StaticBlock
{
    std::string     name;
    uint32_t        imageCount;
    Image*          images;
    int32_t         spawnOdds;
    bool            isSolid;
    int32_t         pickupCount;
    uint16_t        pickupIndex;
    int32_t         breakTimeMS;
    uint16_t        replacementIndex;
};

struct StaticItem
{
    std::string name;
    Image       image;
    bool        isSingle, isTool;
    int16_t     blockIndex = -1;
    int16_t     smeltIndex = -1;
};

struct CraftingRecipe
{
    uint16_t    requirementIndex[MAX_INGREDIENTS];
    uint8_t     requirementCount[MAX_INGREDIENTS];
    Slot        requirements[MAX_INGREDIENTS];
    uint16_t    resultIndex;
    Slot        result;
    enum
    {
        STONE_PICKAXE,
        STONE_AXE,
        CAMPFIRE,
        IRON_PICKAXE,
        IRON_AXE,
        COUNT
    };
};

struct Fireplace
{
    static constexpr int    LOG_VAL = 20'000;
    static constexpr int    LOG_MAX = LOG_VAL * 3;
    static constexpr int    SMELT_TIME = 5'000;
    static constexpr int    ARROW_SPRITE_COUNT = 13;
    int32_t logCount;
    Slot    input;
    Slot    output;
    int32_t tickCount;
};

namespace Entities
{
    inline std::list<Fireplace>     fireplace;
    inline std::list<ItemEntity>    itemEntity;
};

namespace DroppedItem
{
    static constexpr auto   MULTIPLIER = 0.03f;
    static constexpr auto   FRICTION = 0.990f;
}

namespace Statics
{
    inline StaticBlock     block[Block::COUNT];
    inline StaticItem      item[Item::COUNT];
    inline CraftingRecipe  recipe[CraftingRecipe::COUNT];
}

namespace Player
{
    inline Vec2            pos, dir;
    inline bool            facingRight;
    inline Image*          image;
    inline Slot            inventory[INVENTORY_SIZE];
    inline uint8_t         hotbarIndex;
    inline Item            heldItem;
    inline int16_t         heldItemOrigin;
    inline GraphicStr      str;
};

// init
GameInfo	init();
void	    initWindow(uint32_t vramX, uint32_t vramY, uint32_t scrX, uint32_t scrY, std::string windowName, std::string iconPath);
Image*      initTextures(std::string path, uint32_t count, int32_t height = 0);
Image*      initFont(uint32_t height);

// tools
uint32_t    tRand(uint32_t modulo);
float       floatMod1(float f);
float       posToOpenGL(float pos, int max);
void	    setPlayerString(std::string str);
uint64_t    tickGet();
void	    addDroppedItem(Item item, Vec2 mapPos, Vec2 dir, float scalar);

// vector tools
IVec2       toIVec2(Vec2 cpy);
IVec2       toIVec2(Vec2 cpy, int scalar);
Vec2        toVec2(IVec2 cpy);
Vec2        toVec2(IVec2 cpy, int scalar);
Vec2        subtractVec2(Vec2 a, Vec2 b);
Vec2        normaliseVec2(Vec2 a);
bool        AABB(Vec2 position, Vec2 size, Vec2 point);
Vec2        cursorToMap(Vec2 pos);
Vec2        mapToCursor(Vec2 pos);

// movement
LRESULT CALLBACK    windowMessageHandler(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);
void                getMovement(GameInfo* info);
void                getCursorState(HWND window, GameInfo* info);

template <typename T>
T tMin(T v1, T v2) {
    return v1 < v2 ? v1 : v2;
}

template <typename T>
T tMax(T v1, T v2) {
    return v1 > v2 ? v1 : v2;
}

#endif