#include "2DGame.hpp"
#include "Render.hpp"

void    drawString(GameInfo* info, Vec2 pos, std::string str, bool addShadow)
{
    int offset = 0;

    for (int i = 0; str[i]; i++)
    {
        if (addShadow)
        {
            glPixelTransferf(GL_RED_SCALE, 0.0f);
            glPixelTransferf(GL_GREEN_SCALE, 0.0f);
            glPixelTransferf(GL_BLUE_SCALE, 0.0f);
            drawTexture(info, { pos.x + offset + 0.5f, pos.y - 0.5f }, info->screen, &info->font[str[i]]);
            glPixelTransferf(GL_RED_SCALE, 1.0f);
            glPixelTransferf(GL_GREEN_SCALE, 1.0f);
            glPixelTransferf(GL_BLUE_SCALE, 1.0f);
        }
        drawTexture(info, { pos.x + offset, pos.y }, info->screen, &info->font[str[i]]);
        offset += info->font[str[i]].size.x;
    }
}

void    drawCenteredString(GameInfo* info, Vec2 pos, std::string str, bool addShadow)
{
    int offset = 0;

    for (int i = 0; str[i]; i++)
    {
        offset += info->font[str[i]].size.x;
    }
    drawString(info, { pos.x - offset / 2, pos.y }, str, addShadow);
}

void    drawCursorString(GameInfo* info, std::string str, bool addShadow)
{
    drawCenteredString(info, { info->cursor.pos.x, info->cursor.pos.y - 16 }, str, addShadow);
}

void    drawPlayerString(GameInfo* info)
{
    if (Player::str.content != "")
        drawCenteredString(info, { (VRAM_X / 2),  (VRAM_Y / 2) + 8 }, Player::str.content, true);
}

void    drawBottomString(GameInfo* info, std::string str, bool resetRank)
{
    static uint8_t  rank;

    if (resetRank == true)
    {
        rank = 0;
        return;
    }
    drawCenteredString(info, { (float)VRAM_X / 2, 20.0f + rank * 14.0f }, str, true);
}