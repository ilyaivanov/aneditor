void DrawRect(MyBitmap *bitmap, i32 x, i32 y, i32 width, i32 height, i32 color)
{
    // converting math y-axis to screen-based y-axis;
    // y = bitmap->height - y - height;

    if (x + width > bitmap->width)
    {
        width = bitmap->width - x;
    }

    if (y + height > bitmap->height)
    {
        height = bitmap->height - y;
    }

    if (x < 0)
    {
        width = x + width;
        x = 0;
    }
    if (y < 0)
    {
        height = y + height;
        y = 0;
    }

    i32 *row = (i32 *)bitmap->pixels;

    row += y * bitmap->width;

    for (int y = 0; y < height; y += 1)
    {
        i32 *pixel = row + x;
        for (int x = 0; x < width; x += 1)
        {
            *pixel = color;
            pixel += 1;
        }

        row += bitmap->width;
    }
}


inline void DrawTextureTopLeft(MyBitmap *destination, MyBitmap *texture, V2 position, u32 color)
{
    float textX = position.x;
    float textY = position.y;
    u32 width = texture->width;
    u32 height = texture->height;

    if (textX + width > destination->width)
    {
        width = destination->width - textX;
    }

    if (textY >= destination->height)
    {
        return;
    }
    if (textY + height > destination->height)
    {
        height = destination->height - textY;
    }

    if (textX < 0)
    {
        width = textX + width;
        textX = 0;
    }
    if (textY < 0)
    {
        height = textY + height;
        textY = 0;
    }

    u32 *destinationRow = (u32 *)destination->pixels + destination->width * (u32)textY + (u32)textX;
    u32 *sourceRow = (u32*)texture->pixels;

    u32 colorR = (color >> 16) & 0xff;
    u32 colorG = (color >> 8) & 0xff;
    u32 colorB = (color >> 0) & 0xff;

    u32 colorRLinear = colorR * colorR;
    u32 colorGLinear = colorG * colorG;
    u32 colorBLinear = colorB * colorB;

    for (int y = 0; y < height; y += 1)
    {
        u32 *destPixel = destinationRow;
        u32 *sourcePixelPointer = sourceRow;
        for (int x = 0; x < width; x += 1)
        {
            u32 sourcePixelV = *sourcePixelPointer;
            u32 sourceR = (sourcePixelV >> 16) & 0xff;
            u32 sourceG = (sourcePixelV >> 8) & 0xff;
            u32 sourceB = (sourcePixelV >> 0) & 0xff;


            float alphaR = sourceR / 255.0f;  
            float alphaG = sourceG / 255.0f;  
            float alphaB = sourceB / 255.0f;  


            u32 desR = (*destPixel >> 16) & 0xff;
            u32 desG = (*destPixel >> 8) & 0xff;
            u32 desB = (*destPixel >> 0) & 0xff;

            u32 destRLinear = desR * desR; 
            u32 destGLinear = desG * desG; 
            u32 destBLinear = desB * desB; 
            

            u32 resRlinear = destRLinear * (1 - alphaR) + colorRLinear * alphaR;
            u32 resGlinear = destGLinear * (1 - alphaG) + colorGLinear * alphaG;
            u32 resBlinear = destBLinear * (1 - alphaB) + colorBLinear * alphaB;


            u32 resR = (u32)(sqrtf(resRlinear) + 0.5f);
            u32 resG = (u32)(sqrtf(resGlinear) + 0.5f);
            u32 resB = (u32)(sqrtf(resBlinear) + 0.5f);

            u32 res = resR << 16 | resG << 8 | resB << 0;

            *destPixel = res;

            destPixel++;
            sourcePixelPointer++;
        }
        destinationRow += destination->width;
        sourceRow += texture->width;
    }
}