#include <stdlib.h>
#include <stdio.h>
#include "editor.h"
#include "font/font.h"
#include "math.c"
#include "bmp.c"
#include "bitmap.c"

#define BACKGROUND_COLOR_GREY 0x11
#define SCREEN_PADDING 20
#define MAX_LINES 256

int lines[MAX_LINES];
int position = 0;

inline u32 GetCurrentPositionLineIndex()
{
    int currentLineIndex = 0;
    if (position > lines[0])
    {
        for (int i = 0; i < ArrayLength(lines) - 1; i += 1)
        {
            if (lines[i] <= position && (lines[i + 1] > position || lines[i + 1] == 0))
            {
                currentLineIndex = i;
                break;
            }
        }
    }
    return currentLineIndex;
}

void SplitTextIntoLines(FileContent textFile, u32 maxWidth)
{
    memset(&lines, 0, sizeof lines);

    lines[0] = -1;

    int currentLine = 0;
    int runningWidth = 0;
    int wordFirstLetterIndex = 0;
    int wordLength = 0;

    char *letter = (u8 *)textFile.content;

    for (int i = 0; i < textFile.size; i += 1)
    {
        i8 ch = *letter;
        
        if ((ch == ' ' || ch == '\n') && runningWidth >= maxWidth)
        {
            lines[++currentLine] = wordFirstLetterIndex - 1;        
            runningWidth = wordLength;
            wordFirstLetterIndex = i + 1;
            wordLength = 0;

            if (ch == '\n')
            {
                lines[++currentLine] = i;
                runningWidth = 0;
                wordFirstLetterIndex = i + 1;
                wordLength = 0;
            }
        }
        else if (ch == '\n')
        {
            if (runningWidth >= maxWidth)
            {
                lines[++currentLine] = wordFirstLetterIndex - 1;
                runningWidth = wordLength;
                wordFirstLetterIndex = i + 1;
                wordLength = 0;
            }
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

FileContent file;

// used to track screen width change
u32 screenWidth = 0;
void GameInit(MyBitmap *bitmap, MyFrameInput *input)
{
    screenWidth = bitmap->width;
    file = input->readFile("..\\sample.txt");
    // file = input->readFile("..\\small.txt");
    MyAssert(file.size > 0);
    InitFontSystem(12); // 12 corresponds for Segoe UI to browser 16px on my machine. Need to investigate why (probably hardcoded 72 DPI in the code at gdiFont.c)

    int totalWidth = bitmap->width;

    SplitTextIntoLines(file, bitmap->width - SCREEN_PADDING * 2);
}





float totalTime = 0;

void GameUpdateAndRender(MyBitmap *bitmap, MyFrameInput *input, float deltaMs)
{
    if(screenWidth != bitmap->width)
    {
        SplitTextIntoLines(file, bitmap->width - SCREEN_PADDING * 2);
        screenWidth = bitmap->width;
    }

    i8 *fileContent = (i8*)file.content;
    if (input->leftPressedThisFrame && position > 0)
    {
        if (fileContent[position - 1] == '\r')
            position -= 2;
        else
            position -= 1;
    }
    if (input->rightPressedThisFrame && position < file.size)
    {
        if (fileContent[position + 1] == '\r')
            position += 2;
        else
            position += 1;
    }

    if (input->downPressedThisFrame)
    {
        u32 currentLine = GetCurrentPositionLineIndex();
        if (lines[currentLine + 1] == 0)
            position = file.size;
        else
        {
            u32 carretLineOffset = position - lines[currentLine];
            u32 nextPos = lines[currentLine + 1] + carretLineOffset;
            position = nextPos > file.size ? file.size : nextPos;
        }
    }
    if (input->upPressedThisFrame)
    {

        u32 currentLine = GetCurrentPositionLineIndex();
        if (currentLine == 0)
        {
            position = 0;
        }
        else
        {
            u32 nextPos = position - (lines[currentLine] - lines[currentLine - 1]);
            position = nextPos < 0 ? 0 : nextPos;
        }
    }

    int x = SCREEN_PADDING;
    int y = SCREEN_PADDING;

    // first line index
    int currentLineEndIndex = 1;
    int lineHeight = GetFontHeight();
    int paragraphHeight = lineHeight * 1.4;

    char *ch = fileContent;
    for(int i = 0; i < file.size; i += 1)
    {

        if (position == i)
        {
            DrawRect(bitmap, x-1, y, 2, lineHeight, 0xdddddd);
        }

        if (lines[currentLineEndIndex] == i)
        {
            y += *ch == '\n' ? paragraphHeight : lineHeight;
            x = SCREEN_PADDING;
            currentLineEndIndex += 1;
        }
        else if (*ch != '\r')
        {
            char codepoint = *ch;
            MyBitmap *glyphBitmap = GetGlyphBitmap(codepoint);
            DrawTextureTopLeft(bitmap, glyphBitmap, (V2){x, y}, 0xeeeeee);

            x += glyphBitmap->width;
        }
        ch++;
    }

    if (position == file.size)
    {
        DrawRect(bitmap, x-1, y, 2, lineHeight, 0xeeeeee);
    }


    totalTime += deltaMs;
}

