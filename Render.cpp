#include "2DGame.hpp"
#include "Render.hpp"

LRESULT CALLBACK windowMessageHandler(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);
void(_stdcall* glWindowPos2f)(float x, float y);
void(_stdcall* glGenerateMipmap)(unsigned target);
unsigned(_stdcall* wglSwapIntervalEXT)(unsigned status);

// graphic
BITMAPINFO  bmi;
HWND        window;
HDC         WindowDC;
WNDCLASSA   wc;

// openGL
PIXELFORMATDESCRIPTOR pfd;

// my stuff
Pixel* vram;
GameInfo* g_info;

void	initWindow(uint32_t vramX, uint32_t vramY, uint32_t scrX, uint32_t scrY, std::string windowName, std::string iconPath)
{
    // windows
    WNDCLASSA wc{};

    wc.lpfnWndProc = windowMessageHandler;                                                                      // Pointer to the window procedure
    wc.hInstance = GetModuleHandle(NULL);                                                                       // Handle to the application instance
    wc.lpszClassName = "class";                                                                                 // Name of the window class
    wc.hCursor = LoadCursor(NULL, IDC_HAND);                                                                    // Default cursor
    wc.hIcon = (HICON)LoadImageA(NULL, iconPath.c_str(), IMAGE_ICON, 16, 16, LR_DEFAULTSIZE | LR_LOADFROMFILE); // Create Icon
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);                                                              // Default background color
    wc.style = CS_HREDRAW | CS_VREDRAW;                                                                         // Window style (optional)

    ShowCursor(false);

    // Register the window class
    RegisterClassA(&wc);
    vram = new Pixel[vramX * vramY]{};
    window = CreateWindowExA(0, "class", windowName.c_str(), WS_VISIBLE /*| WS_POPUP*/, 0, 0, scrX, scrY, 0, 0, wc.hInstance, 0);
    WindowDC = GetDC(window);

    bmi.bmiHeader.biWidth = vramX;
    bmi.bmiHeader.biHeight = vramY;
    bmi.bmiHeader.biBitCount = sizeof(Pixel) * 8;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biSize = sizeof(BITMAPINFO);

    RECT client;
    GetClientRect(window, &client);
    SetWindowPos(window, 0, 0, 0, scrX + (scrX - client.right), scrY + (scrY - client.bottom), 0);

    // openGL
    pfd = {
    sizeof(PIXELFORMATDESCRIPTOR),1,
    PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,PFD_TYPE_RGBA,
    24,0,0,0,0,0,0,0,0,0,
    0,0,0,0,32,0,0,PFD_MAIN_PLANE,
    0,0,0,0
    };
#pragma comment(lib, "opengl32.lib")
    SetPixelFormat(WindowDC, ChoosePixelFormat(WindowDC, &pfd), &pfd);
    wglMakeCurrent(WindowDC, wglCreateContext(WindowDC));
    SwapBuffers(WindowDC);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPixelZoom(scrX / vramX, scrY / vramY);
    glWindowPos2f = (void(_stdcall*)(float, float))wglGetProcAddress("glWindowPos2f");
    glGenerateMipmap = (void(_stdcall*)(unsigned))wglGetProcAddress("glGenerateMipmap");
    wglSwapIntervalEXT = (unsigned(_stdcall*)(unsigned))wglGetProcAddress("wglSwapIntervalEXT");
    wglSwapIntervalEXT(VSYNC);
}

void    getImageSize(int32_t total, IVec2* size)
{
    if (size->x < 0 && size->y < 0)
        *size = { (int)sqrt(total), (int)sqrt(total) };
    else if (size->x < 0)
        size->x = total / size->y;
    else if (size->y < 0)
        size->y = total / size->x;
}

Pixel*  open32BitBMP(std::string fileName, IVec2* size, bool sendMsg)
{
    HANDLE      file;
    std::string buffer;
    char*       buffer2;
    uint32_t    file_size;
    uint32_t    start_dest;
    Pixel*      image;

    file = CreateFileA(fileName.c_str(), GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file == INVALID_HANDLE_VALUE)
    {
        if (sendMsg)
        {
            MessageBoxA(0, ("Couldn't open" + fileName).c_str(), "Error opening file", MB_OK);
        }
        return NULL;
    }
    ReadFile(file, (LPVOID)buffer.c_str(), 14, 0, 0);
    file_size = *(uint32_t*)(buffer.c_str() + 2);
    start_dest = *(uint32_t*)(buffer.c_str() + 10);
    SetFilePointer(file, start_dest, 0, 0);
    image = new Pixel[(file_size - start_dest) / 4];
    buffer2 = new char[file_size];
    ReadFile(file, buffer2, file_size, 0, 0);
    for (int i = 0; i < (file_size - start_dest) / 4; i++)
    {
        image[i].b = buffer2[i * 4 + 0];
        image[i].g = buffer2[i * 4 + 1];
        image[i].r = buffer2[i * 4 + 2];
        image[i].a = buffer2[i * 4 + 3];
    }
    CloseHandle(file);
    delete[] buffer2;
    getImageSize((file_size - start_dest) / 4, size);
    return (image);
}

void    loadTexture(Image* texture)
{
    glGenTextures(1, &texture->index);
    glBindTexture(GL_TEXTURE_2D, texture->index);
    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->size.x, texture->size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->content);
    glGenerateMipmap(GL_TEXTURE_2D);
}

Image*  initTextures(std::string path, uint32_t count, int32_t height)
{
    Image* images = new Image[count]();

    for (int i = 0; i < count; i++)
    {
        if (height == 0)
            images[i].size = { -1, -1 };
        else
            images[i].size = { -1, height };
        images[i].content = open32BitBMP(path + std::to_string(i) + ".bmp", &images[i].size, true);
        loadTexture(&images[i]);
    }
    return images;
}

Image* initFont(uint32_t height)
{
    Image*  font = new Image[FONT_RANGE]();

    for (int i = 0; i < FONT_RANGE; i++)
    {
        font[i].size = { -1, FONT_SIZE_Y };
        font[i].content = open32BitBMP("Sprites/Font/" + std::to_string(i) + ".bmp", &font[i].size, false);
        if (font[i].content == NULL)
            font[i].content = font[0].content;
    }
    return font;
}

// ------------------------ ^^^ Init ^^^ ---------------------------

void    drawSprite(Vec2 pos, Image* image, bool flipped = false)
{
    float   startX = flipped ? pos.x + image->size.x : pos.x;
    float   endX = flipped ? pos.x : pos.x + image->size.x;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, image->index);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(posToOpenGL(startX, VRAM_X), posToOpenGL(pos.y, VRAM_Y));
    glTexCoord2f(0, 1);
    glVertex2f(posToOpenGL(startX, VRAM_X), posToOpenGL(pos.y + image->size.y, VRAM_Y));
    glTexCoord2f(1, 1);
    glVertex2f(posToOpenGL(endX, VRAM_X), posToOpenGL(pos.y + image->size.y, VRAM_Y));
    glTexCoord2f(1, 0);
    glVertex2f(posToOpenGL(endX, VRAM_X), posToOpenGL(pos.y, VRAM_Y));;
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void    drawTexture(GameInfo* info, Vec2 pos, IVec2 screen, Image* image)
{
    glWindowPos2f((pos.x * screen.x) / VRAM_X, (pos.y * screen.y) / VRAM_Y);
    glDrawPixels(image->size.x, image->size.y, GL_RGBA, GL_UNSIGNED_BYTE, image->content);
}

Image*  getImage(GameInfo* info, uint32_t index)
{
    switch (info->blocks[index].blockIndex)
    {
        case Block::CAMPFIRE:
        {
            Fireplace* fireplace = (Fireplace*)info->blocks[index].entity;
            int8_t     logCount = tMin((fireplace->logCount - 1) / Fireplace::LOG_VAL + 1, Fireplace::LOG_MAX);
            if (fireplace->logCount == 0)
                return &Statics::block[info->blocks[index].blockIndex].images[0];
            if (fireplace->logCount / Fireplace::LOG_VAL >= 3)
                return &Statics::block[info->blocks[index].blockIndex].images[3];
            return &Statics::block[info->blocks[index].blockIndex].images[logCount];
        }
        default:
            return &Statics::block[info->blocks[index].blockIndex].images[info->blocks[index].imageIndex];
    }
}

void    renderEntities()
{
    Vec2   itemPos;

    for (ItemEntity& item : Entities::itemEntity)
    {
        itemPos = { item.pos.x, item.pos.y };
        if (AABB({ -BLOCK_S, -BLOCK_S }, { VRAM_X + BLOCK_S * 2, VRAM_Y + BLOCK_S * 2 }, mapToCursor(itemPos)) == true)
        {
            drawSprite
            (
                mapToCursor({ itemPos.x + 0.5f, itemPos.y + 0.5f }),
                &item.image[0]
            );
        }
    }
}

void    renderSlot(GameInfo* info, Slot slot, bool usesMapPos = false, bool currentSlot = false)
{
    if (usesMapPos == true)
        slot.pos = mapToCursor(slot.pos);
    drawSprite(slot.pos, &info->tileUI[currentSlot]);
    if (slot.item.index != Item::EMPTY)
    {
        drawSprite(slot.pos, &Statics::item[slot.item.index].image);
        if (Statics::item[slot.item.index].isSingle == false)
            drawCenteredString(info, { slot.pos.x + 13, slot.pos.y + 5.0f }, std::to_string(slot.item.count), true);
    }
}

void    renderAll(GameInfo* info)
{
    using namespace Player;

    for (int y = -1, trueY = pos.y - BLOCK_Y / 2; y <= BLOCK_Y; y++, trueY++)
    {
        for (int x = -1, trueX = pos.x - BLOCK_X / 2; x <= BLOCK_X; x++, trueX++)
        {
            drawSprite
            (
                mapToCursor(toVec2({ trueX, trueY })),
                &Statics::block[Block::GRASS].images[info->grassIndex[trueX + trueY * MAP_SIZE]],
                (trueX + trueY) % 2
            );
        }
    }
    renderEntities();
    for (int y = BLOCK_Y + 1, trueY = pos.y + BLOCK_Y / 2 + 2; y >= -2; y--, trueY--)
    {
        if (y == BLOCK_Y / 2 - 1)
        {
            drawSprite({ (VRAM_X - BLOCK_S) / 2, (VRAM_Y - BLOCK_S) / 2 }, image, facingRight == true);
        }
        for (int x = -2, trueX = pos.x - BLOCK_X / 2 - 1; x <= BLOCK_X + 1; x++, trueX++)
        {
            if (info->blocks[trueX + trueY * MAP_SIZE].blockIndex == Block::CAMPFIRE)
            {
                Fireplace*  fireplace = (Fireplace*)info->blocks[trueX + trueY * MAP_SIZE].entity;

                if (fireplace->logCount != 0 || fireplace->input.item.count != 0 || fireplace->output.item.count != 0)
                {
                    renderSlot(info, fireplace->input, true);
                    std::string arrow(1, (char)16 + (fireplace->tickCount % Fireplace::SMELT_TIME) / (Fireplace::SMELT_TIME / Fireplace::ARROW_SPRITE_COUNT));
                    drawString(info, mapToCursor({ fireplace->input.pos.x + 1, fireplace->input.pos.y - 0.1f }), arrow, true);
                    renderSlot(info, fireplace->output, true);
                }
            }
            if (info->blocks[trueX + trueY * MAP_SIZE].blockIndex != Block::GRASS)
            {
                drawSprite
                (
                    mapToCursor(toVec2({ (trueX - 1), trueY })),
                    getImage(info, trueX + trueY * MAP_SIZE),
                    (trueX + trueY) % 2
                );
            }
        }
    }
}

void    renderInventory(GameInfo* info)
{
    using namespace Player;

    Slot*           inv = inventory;

    //inventory
    for (int i = 0; i < INVENTORY_SIZE; i++)
    {
        renderSlot(info, inv[i], false, i == Player::hotbarIndex);
    }
    //crafting menu
    for (int y = 0; y < CraftingRecipe::COUNT; y++)
    {
        int x = 0;

        for (; x < Statics::recipe[y].requirementIndex[x] != Item::EMPTY; x++)
        {
            renderSlot(info, Statics::recipe[y].requirements[x]);
        }
        Vec2    screenPos = Statics::recipe[y].result.pos;

        drawString(info, { screenPos.x - BLOCK_S - 1, screenPos.y - 1}, "\x7F", true);
        renderSlot(info, Statics::recipe[y].result);
    }
}

void    drawCursor(GameInfo* info)
{
    using namespace Player;

    Vec2    cursor = { info->cursor.pos.x - info->crosshair->size.x / 2, info->cursor.pos.y - info->crosshair->size.y / 2 };

    if (heldItem.index == Item::EMPTY)
    {
        if (Statics::item[inventory[hotbarIndex].item.index].blockIndex != -1)
        {
            glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
            drawSprite(mapToCursor(cursorToMap({ cursor.x - 8, cursor.y + 8 })), Statics::block[Statics::item[inventory[hotbarIndex].item.index].blockIndex].images);
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        }
        else
        {
            glColor3f(0.0f, 0.0f, 0.0f);
            drawSprite({ cursor.x + 0.5f, cursor.y - 0.5f }, info->crosshair);
            glColor3f(1.0f, 1.0f, 1.0f);
            drawSprite(cursor, info->crosshair);
        }
    }
    else if (heldItem.index != -1)
    {
        drawSprite(cursor, &Statics::item[heldItem.index].image);
        if (Statics::item[heldItem.index].isSingle == false)
            drawCenteredString(info, { cursor.x + 14, cursor.y + 5.5f }, std::to_string(inventory[heldItem.index].item.count), true);
    }
}

void    toScreen()
{
    SwapBuffers(WindowDC);
}

void    render(GameInfo* info)
{
    MSG     msg;

    g_info = info;
    while (PeekMessageA(&msg, window, 0, 0, 0))
    {
        GetMessageA(&msg, window, 0, 0);
        DispatchMessageA(&msg);
    } 
    renderAll(info);
    drawPlayerString(info);
    if (info->overlay == Overlay::INVENTORY)
        renderInventory(info);
    getCursorState(window, info);
    drawCursor(info);
}
