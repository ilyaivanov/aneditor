#include <stdlib.h>
#include <stdio.h>
#include "editor.h"
#include "math.c"
#include "bmp.c"
#include "bitmap.c"

#define BACKGROUND_COLOR_GREY 0x22
#define FOREGROUND_COLOR 0xFFFFFF
#define FOREGROUND_COLOR_ERROR 0xFF2222

#define CELL_PADDING 4
#define LEFT_PADDING 1
#define TOP_PADDING 1
#define CELL_SIZE 7
#define PIXEL_SIZE 2
#define GRID_WIDTH 16

#define LETTERS_SUPPORTED 256

typedef struct LetterPixels
{
    u32 pointCount;
    
    // 22 is picked by hand based on the bitmap 
    V2i8 points[22];
} LetterPixels;

LetterPixels letters[LETTERS_SUPPORTED];

LetterPixels notFoundPixels;

void GameInit(MyBitmap *bitmap, MyFrameInput *input)
{
    //TODO: memory leak, I will free bitmap once I will convert symbols to C structures
    FileContent file = input->readFile("..\\aseprite_font.bmp");

    MyBitmap fontTexture;
    ParseBmpFile(&file, &fontTexture);

    for (int i = 0; i < LETTERS_SUPPORTED; i += 1)
    {
        int gridY = i / GRID_WIDTH;
        int gridX = i % GRID_WIDTH;

        LetterPixels *pixels = &letters[i];
        u32 *sourceRow = (u32 *)fontTexture.pixels +
                         TOP_PADDING * fontTexture.width +
                         gridY * fontTexture.width * (CELL_PADDING + CELL_SIZE) +
                         LEFT_PADDING + gridX * (CELL_PADDING + CELL_SIZE);

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
                    MyAssert(pixels->pointCount < ArrayLength(pixels->points));

                    u32 pointCount = pixels->pointCount;
                    pixels->points[pointCount].x = x;
                    pixels->points[pointCount].y = y;
                    pixels->pointCount += 1;

                    // DrawRect(bitmap, posX + x * PIXEL_SIZE, posY + y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE, FOREGROUND_COLOR);
                }

                sourcePixel++;
            }

            sourceRow += fontTexture.width;
        }
    }

    //Fill NOT FOUND pixels
    for (int y = 0; y < CELL_SIZE; y += 1)
    {
        for (int x = 0; x < 4; x += 1)
        {
            notFoundPixels.points[notFoundPixels.pointCount].x = x;
            notFoundPixels.points[notFoundPixels.pointCount].y = y;
            notFoundPixels.pointCount += 1;
        }
    }
}

int DrawPixelGlyphAt(MyBitmap *bitmap, u8 ch, int posX, int posY)
{
    // Need to think how to support UTF-16 or even extended ASCI set
    int isCharSupported = ch < 128;

    int codepointIndex = ch - '!' + 1;

    LetterPixels *pixels = isCharSupported ? &letters[codepointIndex] : &notFoundPixels;
    u32 color = isCharSupported  ? FOREGROUND_COLOR : FOREGROUND_COLOR_ERROR;

    int maxX = 0;

    for (int i = 0; i < pixels->pointCount; i += 1)
    {
        int x = pixels->points[i].x;
        int y = pixels->points[i].y;
        DrawRect(bitmap, posX + x * PIXEL_SIZE, posY + y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE, color);

        if (x > maxX)
            maxX = x;
    }

    return maxX + 1;
}

void DrawPixelGrid(MyBitmap *bitmap)
{
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
}

float totalTime = 0;
void GameUpdateAndRender(MyBitmap *bitmap, MyFrameInput *input, float deltaMs)
{
    // DrawPixelGrid(bitmap);

    char *text = "Hello there from the undertale! My monery $500 while me & you pointers * (fo)";
    int step = PIXEL_SIZE * CELL_SIZE;

    int startX = 20;
    int x = 20;
    int y = 20;
    int lineAdvance = 2;
    while (*text)
    {
        u8 ch = *text;
        if (ch == ' ')
        {
            x += 3 * PIXEL_SIZE;
        }
        else if (ch == '\n')
        {
            y += (CELL_SIZE + lineAdvance) * PIXEL_SIZE;
            x = startX;
        }
        else if (ch == '\r')
        {
            // skip windows carrage returns
        }
        else
        {

            x += (DrawPixelGlyphAt(bitmap, ch, x, y) + 1) * PIXEL_SIZE;

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
