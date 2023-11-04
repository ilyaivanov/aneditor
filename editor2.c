#include <stdlib.h>
#include <stdio.h>
#include "editor.h"
#include "font/font.h"
#include "math.c"
#include "bmp.c"
#include "bitmap.c"

#define SHOW_GRID 0
#define BACKGROUND_COLOR_GREY 0x11
#define FOREGROUND_COLOR 0xFFFFFF
#define CURSOR_COLOR 0xFF2222
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

char text[] = "A pointer is a variable that contains the address of a variable. Pointers are much used in C, partly because they are sometimes the only way to express a computation, and partly because they usually lead to more compact and efficient code than can be obtained in other ways. Pointers and arrays are closely related; this chapter also explores this relationship and shows how to exploit it.";
int lines[20];

#define SCREEN_PADDING 20

void GameInit(MyBitmap *bitmap, MyFrameInput *input)
{
    InitFontSystem(11);

    int totalWidth = bitmap->width;

    char *letter = &text[0];

    int currentLine = -1;
    int maxWidth = bitmap->width - SCREEN_PADDING * 2;
    int runningWidth = 0;
    int wordFirstLetterIndex = 0;
    int wordLength = 0;

    for (int i = 0; i < ArrayLength(text) - 1; i += 1)
    {
        i8 ch = *letter;
        
        if (ch == ' ' && runningWidth >= maxWidth)
        {
            lines[++currentLine] = wordFirstLetterIndex - 1;        
            runningWidth = wordLength;
            wordFirstLetterIndex = i + 1;
            wordLength = 0;
        }
        else
        {
            int w = GetGlyphWidth(ch);
            if (ch == ' ')
            {
                wordFirstLetterIndex = i + 1;
                wordLength = 0;
            }
            else
            {
                wordLength += w;
            }
            runningWidth += w;
        }

        letter += 1;
    }
}

float totalTime = 0;
void GameUpdateAndRender(MyBitmap *bitmap, MyFrameInput *input, float deltaMs)
{
    int x = SCREEN_PADDING;
    int y = SCREEN_PADDING;
    char* ch = text;
    int line = 0;
    int lineHeight = GetFontHeight();

    for(int i = 0; i < ArrayLength(text) - 1; i += 1)
    {
        if (lines[line] == i)
        {
            y += lineHeight;
            x = SCREEN_PADDING;
            line += 1;
        }
        else
        {
            char codepoint = *ch;
            MyBitmap *glyphBitmap = GetGlyphBitmap(codepoint);
            DrawTextureTopLeft(bitmap, glyphBitmap, (V2){x, y}, 0xffffff);

            x += glyphBitmap->width;
        }
        ch++;
    }

    totalTime += deltaMs;
}

