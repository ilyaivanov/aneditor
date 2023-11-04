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

int lines[256];

#define SCREEN_PADDING 20

FileContent file;
void GameInit(MyBitmap *bitmap, MyFrameInput *input)
{
    file = input->readFile("..\\sample.txt");
    MyAssert(file.size > 0);
    InitFontSystem(11);

    int totalWidth = bitmap->width;


    int currentLine = -1;
    int maxWidth = bitmap->width - SCREEN_PADDING * 2;
    int runningWidth = 0;
    int wordFirstLetterIndex = 0;
    int wordLength = 0;

    char *letter = (u8 *)file.content;
    for (int i = 0; i < file.size; i += 1)
    {
        i8 ch = *letter;;
        
        if ((ch == ' ' || ch == '\n') && runningWidth >= maxWidth)
        {
            lines[++currentLine] = wordFirstLetterIndex - 1;        
            runningWidth = wordLength;
            wordFirstLetterIndex = i + 1;
            wordLength = 0;
        }
        else if (ch == '\n')
        {
            lines[++currentLine] = i;
            runningWidth = 0;
            wordFirstLetterIndex = i + 1;
            wordLength = 0;
        }
        else if (ch != '\r')
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
    int line = 0;
    int lineHeight = GetFontHeight();
    int paragraphHeight = GetFontHeight() * 1.4;

    char *ch = (u8 *)file.content;
    for(int i = 0; i < file.size; i += 1)
    {
        if (lines[line] == i)
        {
            y += *ch == '\n' ? paragraphHeight : lineHeight;
            x = SCREEN_PADDING;
            line += 1;
        }
        else if (*ch != '\r')
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

