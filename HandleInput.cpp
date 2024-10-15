#include "2DGame.hpp"

Input   g_input;
Cursor* g_cursor;

LRESULT CALLBACK    windowMessageHandler(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (!g_cursor)
        return DefWindowProcA(window, msg, wParam, lParam);
    switch (msg)
    {
        case WM_LBUTTONDOWN:
            if (g_cursor->leftClick == false)
            {
                g_cursor->activeTime = timeGetTime();
                g_cursor->leftClick = true;
            }
            break;
        case WM_LBUTTONUP:
            g_cursor->leftClick = false;
            break;
        case WM_RBUTTONDOWN:
            g_cursor->rightClick = true;
            break;
        case WM_RBUTTONUP:
            g_cursor->rightClick = false;
            break;
        case WM_MOUSEWHEEL:
            g_input.mwDir = GET_WHEEL_DELTA_WPARAM(wParam);
            break;
        case WM_QUIT:
        case WM_CLOSE:
            ExitProcess(0);
    }
    g_input.w = GetKeyState('W') & 0x80;
    g_input.s = GetKeyState('S') & 0x80;
    g_input.d = GetKeyState('D') & 0x80;
    g_input.a = GetKeyState('A') & 0x80;
    g_input.e = GetKeyState('E') & 0x80;
    g_input.numPressed = -1;
    for (int i = 0; i < INVENTORY_WIDTH && i < 10; i++)
    {
        if ((GetKeyState(i + '1') & 0x80))
        {
            g_input.numPressed = i;
        }
    }
    return DefWindowProcA(window, msg, wParam, lParam);
}

void     getCursorState(HWND window, GameInfo* info)
{
    POINT   cursor;
    RECT    client;
    RECT    windowOffset;
    IVec2   blockNow;

    if (!g_cursor)
        g_cursor = &info->cursor;
    GetCursorPos(&cursor);
    ScreenToClient(window, &cursor);
    GetClientRect(window, &client);
    cursor.y = client.bottom - cursor.y;

    info->cursor.pos.x = (float)(cursor.x) * VRAM_X / client.right;
    info->cursor.pos.y = (float)(cursor.y) * VRAM_Y / client.bottom;
    //std::cout << "cursor x: " << info->cursor.pos.x << " y: " << info->cursor.pos.y << "\n";
    blockNow.x = Player::pos.x + info->cursor.pos.x * BLOCK_X / VRAM_X - BLOCK_X / 2 + 1;
    blockNow.y = Player::pos.y + info->cursor.pos.y * BLOCK_Y / VRAM_Y - BLOCK_Y / 2 + 1;
    if (blockNow.x != info->cursor.blockIndex.x || blockNow.y != info->cursor.blockIndex.y)
        info->cursor.activeTime = timeGetTime();
    info->cursor.blockIndex.x = blockNow.x;
    info->cursor.blockIndex.y = blockNow.y;
}

void    getMiscKeypresses(GameInfo* info)
{
    using namespace Overlay;

    static DWORD    lastEPress;

    if (g_input.e && lastEPress + 200 < timeGetTime())
    {
        info->overlay = info->overlay == INVENTORY ? NONE : INVENTORY;
        lastEPress = timeGetTime();
    }
    if (g_input.numPressed != -1)
        Player::hotbarIndex = g_input.numPressed;
    if (g_input.mwDir < 0)
    {
        Player::hotbarIndex = (Player::hotbarIndex + 1) % INVENTORY_WIDTH;
        g_input.mwDir = 0;
    }
    else if (g_input.mwDir > 0)
    {
        Player::hotbarIndex = Player::hotbarIndex > 0 ? Player::hotbarIndex - 1 : INVENTORY_WIDTH - 1;
        g_input.mwDir = 0;
    }
}

void    getMovement(GameInfo* info)
{
    using namespace Player;

    float   move_speed = (g_input.a || g_input.d) && (g_input.w || g_input.s) ? MV_DIAGONAL : MV_SPEED;

    getMiscKeypresses(info);
    if (g_input.a)
        dir.x -= move_speed;
    if (g_input.d)
        dir.x += move_speed;
    if (g_input.w)
        dir.y += move_speed;
    if (g_input.s)
        dir.y -= move_speed;
    if (Statics::block[info->blocks[(int)(pos.x + dir.x + 1) + (int)(pos.y + 1) * MAP_SIZE].blockIndex].isSolid == false)
        pos.x += dir.x;
    if (Statics::block[info->blocks[(int)(pos.x + 1) + (int)(pos.y + dir.y + 1) * MAP_SIZE].blockIndex].isSolid == false)
        pos.y += dir.y;
    if (dir.x > 0)
        facingRight = true;
    else if (dir.x < 0)
        facingRight = false;
    dir.x *= MV_FRICTION;
    dir.y *= MV_FRICTION;
}
