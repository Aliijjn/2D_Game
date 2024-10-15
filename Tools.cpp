#include "2DGame.hpp"

uint32_t tRand(uint32_t modulo)
{
	uint32_t	x = __rdtsc();

	x += (x << 10);
	x ^= (x >> 6);
	x += (x << 3);
	x ^= (x >> 11);
	x += (x << 15);
	return x % modulo;
}

void    countFPS()
{
    static int      frames;
    static DWORD    previousTime;
    DWORD           currentTime;

    frames++;
#pragma comment(lib, "winmm.lib")
    currentTime = timeGetTime();
    if (previousTime % 1000 > currentTime % 1000)
    {
        std::cout << "FPS: " << frames << "\n";
        frames = 0;
    }
    previousTime = currentTime;
}

float   floatMod1(float f)
{
    return f - (int)f;
}

float   posToOpenGL(float pos, int max)
{
    return pos * 2 / max - 1;
}

void	setPlayerString(std::string str)
{
    Player::str = { str, timeGetTime(), MESSAGE_UPTIME };
}

uint64_t    tickGet()
{
    LARGE_INTEGER    frequency, count;

    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&count);
    count.QuadPart /= (frequency.QuadPart / 1000);
    return count.QuadPart;
}

void	addDroppedItem(Item item, Vec2 mapPos, Vec2 dir, float scalar)
{
    ItemEntity entity{};

    entity.pos = { mapPos.x - 0.5f, mapPos.y - 0.5f };
    entity.dir = normaliseVec2(dir);
    entity.dir.x *= scalar;
    entity.dir.y *= scalar;
    entity.item = item;
    entity.image = &Statics::item[item.index].image;
    Entities::itemEntity.push_front(entity);
}