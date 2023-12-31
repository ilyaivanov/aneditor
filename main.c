#include <windows.h>
#include "font\gdiFont.c"
#include "editor.c"

// #define LOG_TIME 1

BITMAPINFO bitmapInfo = {0};
MyBitmap bitmap = {0};
MyFrameInput frameInput = {0};

i32 isRunning = 1;

// used to avoid initial resize event calling UpdateAndRender
i32 hasInitialized = 0;

#define DESIRED_MS_PER_FRAME (1000.0f / 60.0f)

FileContent ReadMyFileImp(char* path){

    FileContent res = {0};
    HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

    LARGE_INTEGER size;
    GetFileSizeEx(file, &size);

    uint32_t fileSize = (uint32_t)size.QuadPart;

    void *buffer = VirtualAlloc(0, fileSize, MEM_COMMIT, PAGE_READWRITE);

    DWORD bytesRead;
    ReadFile(file, buffer, fileSize, &bytesRead, 0);
    CloseHandle(file);

    res.content = buffer;
    res.size = bytesRead;
    return res;
}

void FreeMyMemmoryImp(void *memory)
{
    if (memory)
    {
        VirtualFree(memory, 0, MEM_RELEASE);
    }
}

void OnResize(HWND window){
    RECT rect;
    GetClientRect(window, &rect);

    if (bitmap.pixels)
    {
        VirtualFree(bitmap.pixels, 0, MEM_RELEASE);
    }

    bitmap.width = rect.right - rect.left;
    bitmap.height = rect.bottom - rect.top;
    bitmap.bytesPerPixel = 4;

    bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
    bitmapInfo.bmiHeader.biBitCount = bitmap.bytesPerPixel * 8;
    bitmapInfo.bmiHeader.biWidth = bitmap.width;
    bitmapInfo.bmiHeader.biHeight = -bitmap.height; //makes rows go up, instead of going down by default
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

    i32 size = bitmap.width * bitmap.height * bitmap.bytesPerPixel;
    bitmap.pixels = VirtualAlloc(0, size, MEM_COMMIT, PAGE_READWRITE);
}

void DrawBitmap(HDC dc){
    StretchDIBits(dc,
                  0, 0, bitmap.width, bitmap.height,
                  0, 0, bitmap.width, bitmap.height,
                  bitmap.pixels, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}


LRESULT OnEvent(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_DESTROY)
    {
        isRunning = 0;
        // PostQuitMessage(0);
    }
    else if (message == WM_SIZE)
    {
        OnResize(window);

        if (hasInitialized)
        {
            memset(bitmap.pixels, BACKGROUND_COLOR_GREY, bitmap.height * bitmap.width * bitmap.bytesPerPixel);
            GameUpdateAndRender(&bitmap, &frameInput, DESIRED_MS_PER_FRAME);

            InvalidateRect(window, 0, 1);
        }
    }
    else if (message == WM_PAINT)
    {
        PAINTSTRUCT paint = {0};
        HDC dc = BeginPaint(window, &paint);
        DrawBitmap(dc);
        EndPaint(window, &paint);
    }

    return DefWindowProc(window, message, wParam, lParam);
}
// #define EDITOR_WINDOW_STYLE (WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME)
#define EDITOR_WINDOW_STYLE (WS_OVERLAPPEDWINDOW)

HWND OpenGameWindow(HINSTANCE instance)
{
    WNDCLASS windowClass = {
        .hInstance = instance,
        .lpfnWndProc = OnEvent,
        .lpszClassName = "MyWindow",
        .style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC,
        .hCursor = LoadCursor(0, IDC_ARROW),
    };
    RegisterClassA(&windowClass);

    HDC dc = GetDC(0);
    int screenWidth = GetDeviceCaps(dc, HORZRES);

    int windowWidth = 700;
    int windowHeight = 900;
    HWND window = CreateWindowA(windowClass.lpszClassName, "An Editor", EDITOR_WINDOW_STYLE | WS_VISIBLE,
                                /* x */ screenWidth - windowWidth - 25,
                                /* y */ 25,
                                /* w */ windowWidth,
                                /* h */ windowHeight,
                                0, 0, instance, 0);

    // i32 requestedWidth = CANVAS_WIDTH;
    // i32 requestedHeight = CANVAS_HEIGHT;
    // RECT rect;
    // GetClientRect(window, &rect);

    // i32 widthDifference = CANVAS_WIDTH - (rect.right - rect.left);
    // i32 heightDifference = CANVAS_HEIGHT - (rect.bottom - rect.top);

    // HDC dc = GetDC(window);
    // i32 screenHeight = GetDeviceCaps(dc, VERTRES);
    // i32 screenWidth  = GetDeviceCaps(dc, HORZRES);

    // ReleaseDC(window, dc);

    // i32 windowWidth = CANVAS_WIDTH + widthDifference;
    // i32 windowHeight = CANVAS_HEIGHT + heightDifference;
    // SetWindowPos(window, 0, 0, 0, windowWidth, windowHeight, SWP_NOZORDER);
    // SetWindowPos(window, 0, screenWidth - windowWidth, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

    // RECT rect1;
    // GetClientRect(window, &rect1);

    // Add assertions for the new client height and width

    return window;
}

inline i64 GetPerfFrequency()
{
    LARGE_INTEGER res;
    QueryPerformanceFrequency(&res);
    return res.QuadPart;
}

inline i64 GetPerfCounter()
{
    LARGE_INTEGER res;
    QueryPerformanceCounter(&res);
    return res.QuadPart;
}

i32 isFullscreen = 0;
// https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
WINDOWPLACEMENT prevWindowDimensions = {sizeof(prevWindowDimensions)};
void ToggleFullscreen(HWND window)
{
    DWORD style = GetWindowLong(window, GWL_STYLE);
    if (style & EDITOR_WINDOW_STYLE)
    {
        isFullscreen = 1;
        MONITORINFO monitorInfo = {sizeof(monitorInfo)};
        if (GetWindowPlacement(window, &prevWindowDimensions) &&
            GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &monitorInfo))
        {
            SetWindowLong(window, GWL_STYLE, style & ~EDITOR_WINDOW_STYLE);

            SetWindowPos(window, HWND_TOP,
                         monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
                         monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                         monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        isFullscreen = 0;
        SetWindowLong(window, GWL_STYLE, style | EDITOR_WINDOW_STYLE);
        SetWindowPlacement(window, &prevWindowDimensions);
        SetWindowPos(window, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}


typedef BOOL WINAPI set_process_dpi_aware(void);
typedef BOOL WINAPI set_process_dpi_awareness_context(DPI_AWARENESS_CONTEXT);
static void PreventWindowsDPIScaling()
{
    HMODULE WinUser = LoadLibraryW(L"user32.dll");
    set_process_dpi_awareness_context *SetProcessDPIAwarenessContext = (set_process_dpi_awareness_context *)GetProcAddress(WinUser, "SetProcessDPIAwarenessContext");
    if(SetProcessDPIAwarenessContext)
    {
        SetProcessDPIAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
    }
    else
    {
        set_process_dpi_aware *SetProcessDPIAware = (set_process_dpi_aware *)GetProcAddress(WinUser, "SetProcessDPIAware");
        if(SetProcessDPIAware)
        {
            SetProcessDPIAware();
        }
    }
}

int wWinMain(HINSTANCE instance, HINSTANCE prev, PWSTR cmdLine, int showCode)
{
    PreventWindowsDPIScaling();

    HWND window = OpenGameWindow(instance);


    frameInput.readFile = ReadMyFileImp;
    frameInput.freeMemory = FreeMyMemmoryImp;

    HDC dc = GetDC(window);

    MSG msg;

    timeBeginPeriod(1);

    i64 frequency = GetPerfFrequency();
    i64 startTime = GetPerfCounter();
#ifdef LOG_TIME
    char perfMessageBuffer[256];
#endif

    GameInit(&bitmap, &frameInput);
    hasInitialized = 1;

    while (isRunning)
    {
        // memset(&frameInput, 0, sizeof(frameInput));

        frameInput.isClickedInFrame = 0;
        frameInput.leftPressedThisFrame = 0;
        frameInput.rightPressedThisFrame = 0;
        frameInput.upPressedThisFrame = 0;
        frameInput.downPressedThisFrame = 0;


        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {

            if (msg.message == WM_KEYDOWN){
                if(msg.wParam == 'W') frameInput.wPressed = 1;
                if(msg.wParam == 'S') frameInput.sPressed = 1;
                if(msg.wParam == VK_RIGHT) frameInput.rightPressedThisFrame = 1;
                if(msg.wParam == VK_LEFT)  frameInput.leftPressedThisFrame = 1;
                if(msg.wParam == VK_UP) frameInput.upPressedThisFrame = 1;
                if(msg.wParam == VK_DOWN)  frameInput.downPressedThisFrame = 1;
            }

            if (msg.message == WM_KEYUP){
                if(msg.wParam == 'W') frameInput.wPressed = 0;
                if(msg.wParam == 'S') frameInput.sPressed = 0;

                if(msg.wParam == VK_SPACE) ToggleFullscreen(window);
            }

            if (msg.message == WM_MOUSEMOVE){
                frameInput.mouseX = LOWORD(msg.lParam);
                frameInput.mouseY = HIWORD(msg.lParam);
            }

            if (msg.message == WM_LBUTTONDOWN)
            {
                frameInput.isClickedInFrame = 1;
                frameInput.isMouseDown = 1;
            }

            if (msg.message == WM_LBUTTONUP)
            {
                frameInput.isMouseDown = 0;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // reset pixels
        memset(bitmap.pixels, BACKGROUND_COLOR_GREY, bitmap.height * bitmap.width * bitmap.bytesPerPixel);

        // render pixels on the bitmap
        GameUpdateAndRender(&bitmap, &frameInput, DESIRED_MS_PER_FRAME);
        
        OutputDebugStringA("frame\n");

        // render pixels on the screen
        DrawBitmap(dc);

        float logicTime = (GetPerfCounter() - startTime) * 1000.0f / frequency;
        
        i32 timeToSleep = (i32)(DESIRED_MS_PER_FRAME - logicTime) - 1;

        if(timeToSleep > 0)
        {
            Sleep(timeToSleep);
        }

        i64 loopStartedAt = GetPerfCounter();
        i64 time = loopStartedAt;
        while((time - startTime) * 1000.0f / frequency < DESIRED_MS_PER_FRAME){
            time = GetPerfCounter();
        }

        i64 endTime = time;
        float frameMs = (endTime - startTime) * 1000.0f / (float)frequency;
        if(frameMs > DESIRED_MS_PER_FRAME + 0.001f){
            char perfMessageBuffer[255];
            sprintf_s(perfMessageBuffer, sizeof(perfMessageBuffer), 
                  "Lost frame: %05.5fms\n", frameMs);

            OutputDebugStringA(perfMessageBuffer);
        }
#ifdef LOG_TIME
        float ms = (endTime - startTime) * 1000.0f / (float)frequency;
        float loopingTime = ((endTime - loopStartedAt) * 1000.0f / frequency);

        sprintf_s(perfMessageBuffer, sizeof(perfMessageBuffer), 
                  "%05.2fms   =>    logic:%05.2fms   sleep:%02d   looping:%05.2f \n", 
                     ms,              logicTime,    timeToSleep,        loopingTime);

        OutputDebugStringA(perfMessageBuffer);
#endif
        startTime = endTime;
        
    }
    return 0;
}
