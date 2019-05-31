#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define WINDOWCLASS_NAME "wJxBreakout"

HWND MainWindow;
MSG Message;
WNDCLASSEX WindowClass;

HBRUSH ColorTable[4];
static char LevelData[9*15] = {0};

const char* Level = "444444444"
                    "333333333"
                    "111111111"
                    "222222222";
                    
typedef struct {
    float x, y;
    float xspeed, yspeed;
} T_BALL;
T_BALL Ball;

int Paddle = 228;

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam){
    PAINTSTRUCT ps; RECT rc; HDC hdc;
    switch (nMsg){
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
            break;
        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);
            GetClientRect(hwnd, &rc);
            RECT Block; int i;
            for (i=0; i<9*15; i++){
                if (!LevelData[i]) continue;
                Block.left = ((i%9)*2)+2+(i%9)*60; Block.top = ((i/9)*2)+2+(i/9)*15;
                Block.right = (((i%9)*2)+2+(i%9)*60)+60; Block.bottom = (((i/9)*2)+2+(i/9)*15)+15;
                FillRect(hdc, &Block, ColorTable[LevelData[i]-1]);
            }
            SelectObject(hdc, GetStockObject(WHITE_BRUSH));
            Ellipse(hdc, Ball.x-7, Ball.y-7, Ball.x+7, Ball.y+7);
            Block.left = Paddle; Block.top = 350;
            Block.right = Paddle+100; Block.bottom = 362;
            FillRect(hdc, &Block, GetStockObject(WHITE_BRUSH));
            EndPaint(hwnd, &ps);
            return 0;
            break;
    }
    return DefWindowProc(hwnd, nMsg, wParam, lParam);
}

#define TICKS_PER_SEC 60

inline void Paddle_Invalidation(void){
    RECT rc;
    rc.left = Paddle-8; rc.top = 350;
    rc.right = Paddle+8; rc.bottom = 362;
    InvalidateRect(MainWindow, &rc, TRUE);
    rc.left = Paddle+100-8; rc.right = Paddle+100+8;
    InvalidateRect(MainWindow, &rc, TRUE);
}
inline void Block_Invalidation(int Bid){
    RECT rc;
    rc.left = ((Bid%9)*2)+2+(Bid%9)*60; rc.top = ((Bid/9)*2)+2+(Bid/9)*15;
    rc.right = (((Bid%9)*2)+2+(Bid%9)*60)+60; rc.bottom = (((Bid/9)*2)+2+(Bid/9)*15)+15;
    InvalidateRect(MainWindow, &rc, TRUE);
}
inline int BlockIdAtXY(int x, int y){
    return ((x-2)/62)+((y-2)/17)*9;
}

DWORD __stdcall Logic(void* param){
    for(;;){
        RECT rc;
        Ball.x+=Ball.xspeed;
        Ball.y+=Ball.yspeed;
        
        if (GetAsyncKeyState(0x25) && Paddle>6){
            Paddle-=6;
            Paddle_Invalidation();
        }
        if (GetAsyncKeyState(0x27) && Paddle<450){
            Paddle+=6;
            Paddle_Invalidation();
        }
        
        // COLLISION DETECTION: PADDLE
        if (Ball.y > 344 && Ball.y < 368 && Ball.x > Paddle && Ball.x < Paddle+100){
            Ball.yspeed *= -1;
            Ball.y-=2;
            Ball.xspeed = -3.5f + (7.0f * (Ball.x - Paddle)) / 99.0f;
        }
        // COLLISION DETECTION: LEFT BORDER
        if (Ball.x < 8){
            Ball.xspeed *= -1;
            Ball.x+=2;
        }
        // COLLISION DETECTION: RIGHT BORDER
        if (Ball.x > 552){
            Ball.xspeed *= -1;
            Ball.x-=2;
        }
        // COLLISION DETECTION: UPPER BORDER
        if (Ball.y < 8){
            Ball.yspeed *= -1;
            Ball.y+=2;
        }
        // COLLISION DETECTION: BLOCKS
        if (Ball.y < 255){
            if (LevelData[BlockIdAtXY(Ball.x+8,Ball.y)] && Ball.xspeed>0){
                Ball.xspeed *= -1;
                LevelData[BlockIdAtXY(Ball.x+8,Ball.y)] = 0;
                Block_Invalidation(BlockIdAtXY(Ball.x+8,Ball.y));
            }
            if (LevelData[BlockIdAtXY(Ball.x-8,Ball.y)] && Ball.xspeed<0){
                Ball.xspeed *= -1;
                LevelData[BlockIdAtXY(Ball.x-8,Ball.y)] = 0;
                Block_Invalidation(BlockIdAtXY(Ball.x-8,Ball.y));
            }
            if (LevelData[BlockIdAtXY(Ball.x,Ball.y-8)] && Ball.yspeed<0){
                Ball.yspeed *= -1;
                LevelData[BlockIdAtXY(Ball.x,Ball.y-8)] = 0;
                Block_Invalidation(BlockIdAtXY(Ball.x,Ball.y-8));
            }
            if (LevelData[BlockIdAtXY(Ball.x,Ball.y+8)] && Ball.yspeed>0){
                Ball.yspeed *= -1;
                LevelData[BlockIdAtXY(Ball.x,Ball.y+8)] = 0;
                Block_Invalidation(BlockIdAtXY(Ball.x,Ball.y+8));
            }
        }
        
        if (Ball.y>420){
            Ball.x = 278;
            Ball.y = 200;
            Ball.xspeed=0;
            Ball.yspeed=3;
            Paddle = 228;
            InvalidateRect(MainWindow, NULL, TRUE);
            Sleep(1000);
        }
        
        if (Ball.yspeed<0){
            Ball.yspeed -= 0.001f;
        }else{
            Ball.yspeed += 0.001f;
        }
        rc.left = Ball.x-18; rc.top = Ball.y-18;
        rc.right = Ball.x+18; rc.bottom = Ball.y+18;
        InvalidateRect(MainWindow, &rc, TRUE);
        // printf("\rpX=%.3f pY=%.3f sX=%.3f sY=%.3f", Ball.x, Ball.y, Ball.xspeed, Ball.yspeed);
        Sleep(1000/TICKS_PER_SEC);
    }
}

int WINAPI WinMain(HINSTANCE hThis, HINSTANCE hPrev, LPSTR lpszArgument, int nShow){
    ColorTable[0] = CreateSolidBrush(RGB(255,100,0));
    ColorTable[1] = CreateSolidBrush(RGB(255,0,0));
    ColorTable[2] = CreateSolidBrush(RGB(255,255,0));
    ColorTable[3] = CreateSolidBrush(RGB(0,255,180));
    Ball.x = 100;
    Ball.y = 1000;
    int i;
    for (i=0;i<strlen(Level);i++) LevelData[i] = Level[i] - 0x30;

    memset(&WindowClass, 0, sizeof(WindowClass));
    WindowClass.lpszClassName = WINDOWCLASS_NAME;
    WindowClass.cbSize = sizeof(WNDCLASSEX);
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = MainWndProc;
    WindowClass.hInstance = hThis;
    WindowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WindowClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WindowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    RegisterClassEx(&WindowClass);
    MainWindow = CreateWindow(WINDOWCLASS_NAME, "Taki tam 'Breakout'", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                              GetSystemMetrics(SM_CXSCREEN)/2-278, GetSystemMetrics(SM_CYSCREEN)/2-200, 566, 400,
                              NULL, NULL, hThis, NULL);
    ShowWindow(MainWindow, nShow);
    UpdateWindow(MainWindow);
    
    DWORD thId;
    CreateThread(0,0,Logic,0,0,&thId);
    
    while(GetMessage(&Message,NULL,0,0)){
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }
    return Message.wParam;
}
