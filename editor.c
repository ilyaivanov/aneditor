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
#define CELL_SIZE 7
#define PIXEL_SIZE 4

void GameInit(MyBitmap *bitmap, MyFrameInput *input)
{
    FileContent file = input->readFile("..\\aseprite_font.bmp");
    ParseBmpFile(&file, &fontTexture);
}

void DrawPixelGlyphAt(MyBitmap *bitmap, MyBitmap *tex, int gridX, int gridY)
{
}

void DrawTExture(MyBitmap *bitmap, MyBitmap *tex, int x, int y)
{
    u32 *row = (u32 *)bitmap->pixels + bitmap->width * y + x;
    u32 *sourceRow = (u32 *)tex->pixels;
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
    DrawTExture(bitmap, &fontTexture, 20, 20);
    totalTime += deltaMs;
}
