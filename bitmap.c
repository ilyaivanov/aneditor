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