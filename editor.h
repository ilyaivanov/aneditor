#include <stdint.h>

#define ArrayLength(array) (sizeof(array) / sizeof(array[0]))
#define MyAssert(cond) if (!(cond)) { *(u32*)0 = 0; }



#define i8 int8_t
#define i16 int16_t
#define i32 int32_t
#define i64 int64_t

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t


typedef struct
{
    i64 size;
    void *content;
} FileContent;

typedef FileContent ReadMyFile(char* path);
typedef void FreeMyMemmory(void* memory);

typedef struct
{
    i32 width;
    i32 height;
    i32 bytesPerPixel;
    u32 *pixels;
} MyBitmap;



typedef struct
{
    i32 mouseX;
    i32 mouseY;
    u32 wPressed;
    u32 sPressed;

    i32 isClickedInFrame;
    i32 isMouseDown;

    ReadMyFile* readFile;
    FreeMyMemmory* freeMemory;
} MyFrameInput;