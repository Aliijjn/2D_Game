#ifndef RENDER_HPP
#define RENDER_HPP

#include "2DGame.hpp"

// render
void    render(GameInfo* info);
void    countFPS();
void    toScreen();

// render strings
void    drawTexture(GameInfo* info, Vec2 pos, IVec2 screen, Image* image);
void    drawString(GameInfo* info, Vec2 pos, std::string str, bool addShadow);
void    drawCenteredString(GameInfo* info, Vec2 pos, std::string str, bool addShadow);
void    drawCursorString(GameInfo* info, std::string str, bool addShadow = true);
void    drawPlayerString(GameInfo* info);
void    drawBottomString(GameInfo* info, std::string str, bool resetRank = false);

#endif // !RENDER_HPP