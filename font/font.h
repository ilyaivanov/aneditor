#ifndef FONT_H
#define FONT_H

#include "../editor.h"

void InitFontSystem(int fontSize);

MyBitmap* GetGlyphBitmap(char codepoint);

u8 GetGlyphWidth(char codepoint);

int GetFontHeight();

#endif