#include <math.h>

#define Pi 3.14159265359f

typedef struct
{
    float x;
    float y;
} V2;

typedef struct V2i8 { int x,y; } V2i8;

inline float VecTranspose(V2 vec1, V2 vec2)
{
    return vec1.x * vec2.x + vec1.y * vec2.y;
}

inline V2 VecMult(V2 vec, float scalar)
{
    V2 res = {0};
    res.x = vec.x * scalar;
    res.y = vec.y * scalar;
    return res;
}

inline V2 VecAdd(V2 vec1, V2 vec2)
{
    V2 res = {0};
    res.x = vec1.x + vec2.x;
    res.y = vec1.y + vec2.y;
    return res;
}

inline V2 VecDiff(V2 vec1, V2 vec2)
{
    V2 res = {0};
    res.x = vec1.x - vec2.x;
    res.y = vec1.y - vec2.y;
    return res;
}

inline V2 VecDiff3(V2 vec1, V2 vec2, V2 vec3)
{
    V2 res = {0};
    res.x = vec1.x - vec2.x - vec3.x;
    res.y = vec1.y - vec2.y - vec3.y;
    return res;
}

inline void VecNormalize(V2 *vec)
{
    float length = sqrtf(vec->x * vec->x + vec->y * vec->y);
    vec->x /= length;
    vec->y /= length;
}

inline float VecLenSquared(V2 vec)
{
    return vec.x * vec.x + vec.y * vec.y;
}

inline float VecLen(V2 vec)
{
    return sqrtf(vec.x * vec.x + vec.y * vec.y);
}

inline V2 VecAdd3(V2 vec1, V2 vec2, V2 vec3)
{
    V2 res = {0};
    res.x = vec1.x + vec2.x + vec3.x;
    res.y = vec1.y + vec2.y + vec3.y;
    return res;
}

inline V2 VecPerp(V2 vec)
{
    V2 res = {0};
    res.x = -vec.y;
    res.y = vec.x;
    return res;
}

inline float Sin(float v)
{
    return sinf(v);
}

inline float Cos(float v)
{
    return cosf(v);
}

inline float lerp(float a, float b, float t){
    return (1 - t) * a + t * b;
}


inline float Abs(float v)
{
    return v < 0 ? -v : v;
}

inline i32 Round(float v)
{
    return (i32)(v + 0.5f);
}

inline V2 VecLerp(V2 v0, V2 v1, float t)
{
  return VecAdd(VecMult(v0, 1 - t), VecMult(v1, t));//(1 - t) * v0 + t * v1;
}


inline u32 GetShift(u32 value)
{
    u32 res = 0;
    _BitScanForward((DWORD *)&res, value);
    return res;

    // if (!value)
    //     return 0;

    // while (!(value & 1))
    // {
    //     res += 1;
    //     value = value >> 1;
    // }

    // return res;
}