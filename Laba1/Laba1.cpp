#include <Windows.h>
#include "resource.h"

#pragma comment(lib, "msimg32.lib")

#define TIMER 1
#define BACKGROUND_COLOR COLOR_3DSHADOW
#define WM_LOAD_SPRITE WM_USER
#define WM_UPDATE_SPRITE WM_USER + 1

constexpr auto WINDOW_NAME = L"Laba1";
constexpr auto WINDOW_WIDTH = 800;
constexpr auto WINDOW_HEIGHT = 600;
constexpr auto TIMER_INTERVAL = 10000;
constexpr auto AUTO_SPEED = 100;
constexpr auto RAND_COUNT = 4;
constexpr auto SPRITE_STEP = 30;

// Processes messages procedure
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Get current window size
SIZE GetWindowSize(HWND hWnd)
{
    // Get sprite size
    RECT wndRectangle;
    GetClientRect(hWnd, &wndRectangle);

    // Set sprite size
    SIZE wndSize;
    wndSize.cx = wndRectangle.right - wndRectangle.left;
    wndSize.cy = wndRectangle.bottom - wndRectangle.top;

    return wndSize;
}

// Get current sprite size
SIZE GetSpriteSize(HBITMAP hBitmap)
{
    // Get sprite size
    BITMAP sprite;
    GetObject(hBitmap, sizeof(BITMAP), &sprite);

    // Set sprite size
    SIZE spriteSize;
    spriteSize.cx = sprite.bmWidth;
    spriteSize.cy = sprite.bmHeight;

    return spriteSize;
}

// Change Sprite position
COORD ChangeSpritePosition(COORD spritePosition, COORD spriteSteps, HWND hWnd, HBITMAP sprite, BOOL mouse = FALSE)
{
    // Default initialization for sprite position, window size and prite size
    COORD newSpritePosition;
    SIZE wndSize = GetWindowSize(hWnd);
    SIZE spriteSize = GetSpriteSize(sprite);

    // Change X-coord
    if (mouse) {
        newSpritePosition.X = spriteSteps.X;
    }
    else {
        newSpritePosition.X = spritePosition.X + spriteSteps.X;
    }
    
    if (newSpritePosition.X < 0) {
        newSpritePosition.X = 0 + SPRITE_STEP;
    }
    else if (newSpritePosition.X + spriteSize.cx > wndSize.cx) {
        newSpritePosition.X = (SHORT)(wndSize.cx - spriteSize.cx - SPRITE_STEP);
    }

    // Change Y-coord
    if (mouse) {
        newSpritePosition.Y = spriteSteps.Y;
    }
    else {
        newSpritePosition.Y = spritePosition.Y + spriteSteps.Y;
    }
    if (newSpritePosition.Y < 0) {
        newSpritePosition.Y = 0 + SPRITE_STEP;
    }
    else if (newSpritePosition.Y + spriteSize.cy > wndSize.cy) {
        newSpritePosition.Y = (SHORT)(wndSize.cy - spriteSize.cy - SPRITE_STEP);
    }

    return newSpritePosition;
}

// Sprite moves up
COORD UpSteps()
{
    COORD steps;

    steps.X = 0;
    steps.Y = -SPRITE_STEP;

    return steps;
}

// Sprite moves down
COORD DownSteps()
{
    COORD steps;

    steps.X = 0;
    steps.Y = SPRITE_STEP;

    return steps;
}

// Sprite moves left
COORD LeftSteps()
{
    COORD steps;

    steps.X = -SPRITE_STEP;
    steps.Y = 0;

    return steps;
}

// Sprite moves right
COORD RightSteps()
{
    COORD steps;

    steps.X = SPRITE_STEP;
    steps.Y = 0;

    return steps;
}

// Sprite doesn't move
COORD NoSteps()
{
    COORD steps;

    steps.X = 0;
    steps.Y = 0;

    return steps;
}

// Mouse move
COORD MouseSteps(LPARAM lParam)
{
    POINTS pt;
    pt = MAKEPOINTS(lParam);

    COORD steps;
    steps.X = pt.x;
    steps.Y = pt.y;

    return steps;
}

// Auto move
COORD AutoSteps()
{
    // Get random number
    int stepId = rand() % RAND_COUNT;

    // Choose step side
    switch (stepId)
    {
    case 0:
        return UpSteps();
    case 1:
        return DownSteps();
    case 2:
        return LeftSteps();
    case 3:
        return RightSteps();
    }
}

// Fill window with color
int FillWindow(HWND hWnd, COLORREF color)
{
    // Get window rectangle
    RECT wndRectangle;
    GetClientRect(hWnd, &wndRectangle);

    // Get window and brush DC
    HDC wndDC = GetDC(hWnd);
    HBRUSH brush = CreateSolidBrush(color);

    // Fill window with color
    int res = FillRect(wndDC, &wndRectangle, brush);

    // Flush DC
    DeleteObject(brush);
    ReleaseDC(hWnd, wndDC);

    return res;
}

// Put sprite on window
bool PutSprite(HWND hWnd, HBITMAP sprite, COORD coord)
{
    // Get DC for window and sprite
    HDC wndDC = GetDC(hWnd);
    HDC spriteDC = CreateCompatibleDC(wndDC);

    // Save old sprite object
    HGDIOBJ oldObject = SelectObject(spriteDC, sprite);

    // Get bitmap size
    SIZE bmpSize = GetSpriteSize(sprite);

    // Draw sprite
    bool res = TransparentBlt(
        wndDC,
        coord.X,
        coord.Y,
        bmpSize.cx,
        bmpSize.cy,
        spriteDC,
        0,
        0,
        bmpSize.cx,
        bmpSize.cy,
        RGB(255, 255, 255)
    );               

    // Flush DC
    DeleteDC(spriteDC);
    ReleaseDC(hWnd, wndDC);

    return res;
}

// Register a new window class
ATOM RegisterWindowClass(HINSTANCE hInstance)
{
    WNDCLASSEX wndClassEx;

    // Initialize window class fields
    wndClassEx.cbSize = sizeof(WNDCLASSEX);
    wndClassEx.hInstance = hInstance;
    wndClassEx.lpfnWndProc = WndProc;
    wndClassEx.cbClsExtra = 0;
    wndClassEx.cbWndExtra = 0;
    wndClassEx.lpszClassName = WINDOW_NAME;
    wndClassEx.style = CS_HREDRAW | CS_VREDRAW;
    wndClassEx.hIcon = LoadIcon(0, IDI_WINLOGO);
    wndClassEx.hCursor = LoadCursor(0, IDC_ARROW);
    wndClassEx.hbrBackground = (HBRUSH)(BACKGROUND_COLOR);
    wndClassEx.lpszMenuName = 0;
    wndClassEx.hIconSm = 0;

    // Register new window class
    return RegisterClassEx(&wndClassEx);
}

// User message on sprite load
bool LoadSpriteMessage(HWND hWnd, HINSTANCE hInstance)
{
    return PostMessage(hWnd, WM_LOAD_SPRITE, NULL, (LPARAM)hInstance);
}

// User message on sprite update
bool UpdateSpriteMessage(HWND hWnd)
{
    return PostMessage(hWnd, WM_UPDATE_SPRITE, NULL, NULL);
}

// Try initialize a window class instance
BOOL InitWindowInstance(HINSTANCE hInstance, int showMode)
{
    HWND hWnd;

    // Create window with user parameters
    hWnd = CreateWindow(
        WINDOW_NAME,
        WINDOW_NAME,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    // Check result
    if (!hWnd) {
        return FALSE;
    } 

    // Show window
    ShowWindow(hWnd, showMode);

    // Set timer for sprite auto move
    SetTimer(hWnd, TIMER, TIMER_INTERVAL, NULL);

    // Load default sprite.
    LoadSpriteMessage(hWnd, hInstance);
  
    return TRUE;
}

// Processes messages procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Dafault initialization for sprite
    static BOOL flag = FALSE;
    static HBITMAP sprite = NULL;
    static COORD spritePos = { 0, 0 };

    // Processes messages sent to a window
    switch (message)
    {
    // On sprite load
    case WM_LOAD_SPRITE:
        sprite = LoadBitmap((HINSTANCE)lParam, MAKEINTRESOURCE(IDB_SPRITE));
        if (sprite != NULL) {
            UpdateSpriteMessage(hWnd);
        }
        break;
    // On sprite update
    case WM_UPDATE_SPRITE:
        FillWindow(hWnd, GetSysColor(BACKGROUND_COLOR));
        PutSprite(hWnd, sprite, spritePos);
        break;
    // On window size change
    case WM_SIZE:
        spritePos = ChangeSpritePosition(spritePos, NoSteps(), hWnd, sprite);
        UpdateSpriteMessage(hWnd);
        break;
    // On auto move
    case WM_TIMER:
        SetTimer(hWnd, TIMER, AUTO_SPEED, NULL);
        spritePos = ChangeSpritePosition(spritePos, AutoSteps(), hWnd, sprite);
        UpdateSpriteMessage(hWnd);
        break;
    // On window close
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    // On key down
    case WM_KEYDOWN:
        KillTimer(hWnd, TIMER);
        switch (wParam)
        {
        // Sprite moves up
        case VK_UP:
            spritePos = ChangeSpritePosition(spritePos, UpSteps(), hWnd, sprite);
            UpdateSpriteMessage(hWnd);
            break;
        // Sprite moves down
        case VK_DOWN:
            spritePos = ChangeSpritePosition(spritePos, DownSteps(), hWnd, sprite);
            UpdateSpriteMessage(hWnd);
            break;
        // Sprite moves left
        case VK_LEFT:
            spritePos = ChangeSpritePosition(spritePos, LeftSteps(), hWnd, sprite);
            UpdateSpriteMessage(hWnd);
            break;
        // Sprite moves right
        case VK_RIGHT:
            spritePos = ChangeSpritePosition(spritePos, RightSteps(), hWnd, sprite);
            UpdateSpriteMessage(hWnd);
            break;
        // Other keys
        default:
            break;
        }
        SetTimer(hWnd, TIMER, TIMER_INTERVAL, NULL);
        break;
    // On mouse wheel move
    case WM_MOUSEWHEEL:
        KillTimer(hWnd, TIMER);
        // If "shift" is pressed
        if (GET_KEYSTATE_WPARAM(wParam) != MK_SHIFT) {
            if (GET_WHEEL_DELTA_WPARAM(wParam) > 0) {
                // Sprite moves up
                spritePos = ChangeSpritePosition(spritePos, UpSteps(), hWnd, sprite);
                UpdateSpriteMessage(hWnd);
            }
            else {
                // Sprite moves down
                spritePos = ChangeSpritePosition(spritePos, DownSteps(), hWnd, sprite);
                UpdateSpriteMessage(hWnd);
            }
        }
        // If "shift" isn't pressed
        else {
            if (GET_WHEEL_DELTA_WPARAM(wParam) > 0) {
                // Sprite moves left
                spritePos = ChangeSpritePosition(spritePos, LeftSteps(), hWnd, sprite);
                UpdateSpriteMessage(hWnd);
            }
            else {
                // Sprite moves right
                spritePos = ChangeSpritePosition(spritePos, RightSteps(), hWnd, sprite);
                UpdateSpriteMessage(hWnd);
            }
        }
        SetTimer(hWnd, TIMER, TIMER_INTERVAL, NULL);
        break;
    // On mouse lButton down
    case WM_LBUTTONDOWN:
        KillTimer(hWnd, TIMER);
        flag = true;
        break;
    // On mouse move
    case WM_MOUSEMOVE:
        if (flag) {
            spritePos = ChangeSpritePosition(spritePos, MouseSteps(lParam), hWnd, sprite, TRUE);
            UpdateSpriteMessage(hWnd);
        }
        break;
    // On mouse lButton up
    case WM_LBUTTONUP:
        flag = false;
        SetTimer(hWnd, TIMER, TIMER_INTERVAL, NULL);
        break;
    // Other messages
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Main function
int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR cmdLine, _In_ int showMode)
{
    MSG msg;

    // Register new window class
    RegisterWindowClass(hInstance);

    // Try initialize window class instance
    if (!InitWindowInstance(hInstance, showMode))
        return FALSE;

    // Main message processing cycle
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
