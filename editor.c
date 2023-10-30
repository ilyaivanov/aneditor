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
#define GLYPH_ERROR_WIDTH 4
#define SPACE_WIDTH 3
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

FileContent txtFile;

void GameInit(MyBitmap *bitmap, MyFrameInput *input)
{
    FileContent file = input->readFile("..\\aseprite_font.bmp");
    txtFile = input->readFile("..\\sample.txt");

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
            // need to have ability to go outside of the CELL_SIZE bounds, currently @ symbol is not properly drawn (it is widen than CELL_SIZE)
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

    input->freeMemory(file.content);

    //Fill NOT FOUND pixels
    for (int y = 0; y < CELL_SIZE; y += 1)
    {
        for (int x = 0; x < GLYPH_ERROR_WIDTH; x += 1)
        {
            notFoundPixels.points[notFoundPixels.pointCount].x = x;
            notFoundPixels.points[notFoundPixels.pointCount].y = y;
            notFoundPixels.pointCount += 1;
        }
    }
}

// Need to think how to support UTF-16 or even extended ASCI set
#define IsGlyphSupported(ch) (ch) < 128

int GetGlyphWidth(u8 ch){
    
    int isCharSupported = IsGlyphSupported(ch);

    if (!isCharSupported)
        return GLYPH_ERROR_WIDTH;

    int codepointIndex = ch - '!' + 1;
    LetterPixels *pixels = &letters[codepointIndex];
    int maxX = 0;
    for (int i = 0; i < pixels->pointCount; i += 1)
    {
        int x = pixels->points[i].x;
        if (x > maxX)
            maxX = x;
    }

    return maxX + 1;
}

//return the width of a glyph in glyph pixels
int DrawPixelGlyphAt(MyBitmap *bitmap, u8 ch, int posX, int posY)
{
    int isCharSupported = IsGlyphSupported(ch);

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
    int step = PIXEL_SIZE * CELL_SIZE;
    int pagePadding = 20;
    int maxTextWidth = bitmap->width - pagePadding * 2;

    u32 softLineBreaks[200] = {0};
    u32 currentSoftLine = 0;

    // int rectWidth = input->mouseX - pagePadding;
    // if(rectWidth < 150)
    //  rectWidth = 310;

    // DrawRect(bitmap, pagePadding, pagePadding, rectWidth, 2000, 0x335533);

    u32 currentSoftlineWidth = 0;
    i32 currentWord = 0;
    i32 currentWordWidth = 0;

    for (int i = 0; i < txtFile.size; i += 1)
    {
        i8 ch = *((i8 *)txtFile.content + i);
        if (ch == ' ')
        {
            if (currentSoftlineWidth >= maxTextWidth)
            {
                softLineBreaks[currentSoftLine] = currentWord;
                // I don't know why I need - 1 here. Visually new lines may go one logical pixel over the board
                currentSoftlineWidth = currentWordWidth + (SPACE_WIDTH - 1) * PIXEL_SIZE;
                currentSoftLine++;
                currentWord = i;
                currentWordWidth = 0;
            }
            else
            {
                currentWord = i;
                currentWordWidth = 0;
                currentSoftlineWidth += SPACE_WIDTH * PIXEL_SIZE;
            }
        }
        else if (ch == '\n' || i == txtFile.size - 1)
        {
            if (currentSoftlineWidth >= maxTextWidth)
            {
                softLineBreaks[currentSoftLine] = currentWord;
                currentSoftlineWidth = currentSoftlineWidth;
            }
            else
            {
                softLineBreaks[currentSoftLine] = i;
                currentSoftlineWidth = 0;
            }

            currentSoftLine++;
            
            currentWord = i;
            currentWordWidth = 0;
        }
        else
        {
            i32 gWidth = (GetGlyphWidth(ch) + 1) * PIXEL_SIZE;
            currentWordWidth += gWidth;
            currentSoftlineWidth += gWidth;
        }
    }

    u32 currentDrawingLine = 0;

    int x = pagePadding;
    int y = pagePadding;
    int lineAdvance = 3;
    int paragraphAdvance = 8;
    for(int i = 0; i < txtFile.size; i+=1)
    {
        u8 ch = *((u8 *)txtFile.content + i);
        if (i == softLineBreaks[currentDrawingLine])
        {
            if (ch == '\n')
            {
                y += (CELL_SIZE + paragraphAdvance) * PIXEL_SIZE;
            }
            else
            {
                y += (CELL_SIZE + lineAdvance) * PIXEL_SIZE;
            }
            currentDrawingLine += 1;
            x = pagePadding;
        }
        else if (ch == ' ')
        {
            x += SPACE_WIDTH * PIXEL_SIZE;
        }
        else if (ch == '\r')
        {
            // skip windows carrage returns
        }
        else
        {
            x += (DrawPixelGlyphAt(bitmap, ch, x, y) + 1) * PIXEL_SIZE;
        }
    }

    totalTime += deltaMs;
}
