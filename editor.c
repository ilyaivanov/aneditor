#include <stdlib.h>
#include <stdio.h>
#include "editor.h"
#include "math.c"
#include "bmp.c"
#include "bitmap.c"

#define BACKGROUND_COLOR_GREY 0x22

MyBitmap fontTexture;
#define CELL_PADDING 4
#define LEFT_PADDING 1
#define TOP_PADDING 1
#define CELL_SIZE 7
#define PIXEL_SIZE 3
#define GRID_WIDTH 16

void GameInit(MyBitmap *bitmap, MyFrameInput *input)
{
    FileContent file = input->readFile("..\\aseprite_font.bmp");
    ParseBmpFile(&file, &fontTexture);
}

int DrawPixelGlyphAt(MyBitmap *bitmap, MyBitmap *tex, int gridX, int gridY, int posX, int posY)
{
    u32 *sourceRow = (u32 *)tex->pixels +
                     TOP_PADDING * tex->width +
                     gridY * tex->width * (CELL_PADDING + CELL_SIZE) +
                     LEFT_PADDING + gridX * (CELL_PADDING + CELL_SIZE);

    int maxX = 0;
    for (int y = 0; y < CELL_SIZE; y += 1)
    {
        u32 *sourcePixel = sourceRow;
        // need to have ability to go outside of the CELL_SIZE bounds
        for (int x = 0; x < CELL_SIZE; x += 1)
        {
            u32 texturePixel = *sourcePixel;
            u32 alpha = texturePixel & 0xff << 24;
            u32 g = texturePixel & 0xff;
            if (alpha > 0 && g == 0)
            {
                DrawRect(bitmap, posX + x * PIXEL_SIZE, posY + y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE, 0xffffff);

                if(x > maxX) maxX = x;
            }

            sourcePixel++;
        }

        sourceRow += tex->width;
    }

    return maxX + 1;
}

void DrawTExture(MyBitmap *bitmap, MyBitmap *tex, int x, int y)
{
    u32 *row = bitmap->pixels + bitmap->width * y + x;
    u32 *sourceRow = tex->pixels;
    for (int y = 0; y < tex->height; y += 1)
    {
        u32 *pixel = row;
        u32 *sourcePixel = sourceRow;
        for (int x = 0; x < tex->width; x += 1)
        {
            u32 texturePixel = *sourcePixel;
            if (texturePixel > 0)
            {
                // no alpha blending
                // no out of bounds checks
                *pixel = texturePixel;
            }

            pixel++;
            sourcePixel++;
        }
        row += bitmap->width;
        sourceRow += tex->width;
    }
}

float totalTime = 0;

void GameUpdateAndRender(MyBitmap *bitmap, MyFrameInput *input, float deltaMs)
{
    // drawing grid for proper text measurements
    int widthInCells = bitmap->width / PIXEL_SIZE + 1;
    int heightInCells = bitmap->height / PIXEL_SIZE + 1;
    for (int y = 0; y < heightInCells; y += 1)
    {
        for (int x = 0; x < widthInCells; x += 1)
        {
            u32 color = (x + y) % 2 == 0 ? 0x222222 : 0x444444;
            DrawRect(bitmap, x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE, color);
        }
    }

    char *text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
    int step = PIXEL_SIZE * CELL_SIZE;

    int startX = 20;
    int x = 20;
    int y = 20;
    int lineAdvance = 2;
    while (*text)
    {
        if (*text == ' ')
        {
            x += 3 * PIXEL_SIZE;
        }
        else
        {

            int pos = *text - '!' + 1;
            int gridY = pos / GRID_WIDTH;
            int gridX = pos % GRID_WIDTH;

            x += (DrawPixelGlyphAt(bitmap, &fontTexture, gridX, gridY, x, y) + 1) * PIXEL_SIZE;

            if (x + CELL_SIZE * PIXEL_SIZE > bitmap->width - startX * 2)
            {
                x = startX;
                y += (CELL_SIZE + lineAdvance) * PIXEL_SIZE;
            }
        }

        text++;
    }

    totalTime += deltaMs;
}
