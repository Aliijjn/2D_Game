#include "2DGame.hpp"

IVec2   toIVec2(Vec2 cpy)
{
	return { (int)cpy.x, (int)cpy.y };
}

IVec2   toIVec2(Vec2 cpy, int scalar)
{
	return { (int)(cpy.x * scalar), (int)(cpy.y * scalar) };
}

Vec2   toVec2(IVec2 cpy)
{
	return { (float)cpy.x, (float)cpy.y };
}

Vec2   toVec2(IVec2 cpy, int scalar)
{
	return { (float)(cpy.x * scalar), (float)(cpy.y * scalar) };
}

Vec2    subtractVec2(Vec2 a, Vec2 b)
{
    return { a.x - b.x, a.y - b.y };
}

Vec2    normaliseVec2(Vec2 a)
{
    float   totalLen = sqrtf(a.x * a.x + a.y * a.y);

    return { a.x / totalLen, a.y / totalLen };
}

bool    AABB(Vec2 position, Vec2 size, Vec2 point)
{
    bool x = point.x > position.x && point.x < position.x + size.x;
    bool y = point.y > position.y && point.y < position.y + size.y;
    return x && y;
}

Vec2   cursorToMap(Vec2 pos)
{
    Vec2   result;

    result.x = (pos.x / BLOCK_S + Player::pos.x - BLOCK_X / 2 + 1);
    result.y = (pos.y / BLOCK_S + Player::pos.y - BLOCK_Y / 2 + 1);
    //std::cout << "-> map " << result.x << ", " << result.y << "\n";
    return result;
}

Vec2    mapToCursor(IVec2 pos)
{
    Vec2    result;

    result.x = ((float)pos.x - Player::pos.x + BLOCK_X / 2 - 1) * BLOCK_S;
    result.y = ((float)pos.y - Player::pos.y + BLOCK_Y / 2 - 1) * BLOCK_S;
    //std::cout << "-> cursor " << result.x << ", " << result.y << "\n";
    return result;
}

Vec2    mapToCursor(Vec2 pos)
{
    Vec2    result;

    result.x = ((float)pos.x - Player::pos.x + BLOCK_X / 2 - 1) * BLOCK_S;
    result.y = ((float)pos.y - Player::pos.y + BLOCK_Y / 2 - 1) * BLOCK_S;
    //std::cout << "-> cursor " << result.x << ", " << result.y << "\n";
    return result;
}