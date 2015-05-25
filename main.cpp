#include <windows.h>
#include <process.h>
#include <stdio.h>
#include "GUI.h"

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
char szClassName[ ] = "DoubleSnakeSimulator";


static MapBasic mapBasic;
static Snake snake0,snake1;
static struct{
CRITICAL_SECTION displock;
bool refresh;
}thread_comm;

static void drawBox(HDC hdc,int x,int y);
static void drawRect(HDC hdc,COLORREF color,int x1,int y1,int x2,int y2);
static void drawSnake(HDC hdc,COLORREF color,Snake &snake);
static void keyboard(WPARAM wParam);

static void ThreadFunction(void* pParam)
{
    runMain();
    _endthread();
}

int WINAPI WinMain (HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszArgument,
                    int nCmdShow)
{
    extern_init();

    InitializeCriticalSection(&(thread_comm.displock));
    //创建线程
    _beginthread(ThreadFunction,0,NULL);

    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;

    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
               0,                   /* Extended possibilites for variation */
               szClassName,         /* Classname */
               "Double Snake Game Simulator",       /* Title Text */
               WS_OVERLAPPEDWINDOW, /* default window */
               CW_USEDEFAULT,       /* Windows decides the position */
               CW_USEDEFAULT,       /* where the window ends up on the screen */
               SCREEN_W,                 /* The programs width */
               SCREEN_H,                 /* and height in pixels */
               HWND_DESKTOP,        /* The window is a child-window to desktop */
               NULL,                /* No menu */
               hThisInstance,       /* Program Instance handler */
               NULL                 /* No Window Creation data */
           );

    /* Make the window visible on the screen */
    ShowWindow (hwnd, nCmdShow);

    SetTimer(hwnd,1,REPAINT_INTERVAL,NULL);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    DeleteCriticalSection(&(thread_comm.displock));
    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}


/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int init;
    static HDC hdcMem;
    static HBITMAP hbmMem;

    switch (message)                  /* handle the messages */
    {

    case WM_CREATE:

        break;
    case WM_TIMER:
        DISPLOCK();
        if(thread_comm.refresh)
        {
            thread_comm.refresh=false;
            RedrawWindow(hwnd,NULL,NULL,RDW_ERASE|RDW_INVALIDATE);
        }
        DISPUNLOCK();
        break;

    case WM_ERASEBKGND:
        return 0;

    case WM_PAINT:
    {

        PAINTSTRUCT ps;
        HDC hdc;
     //   RECT lprc;
        HANDLE  hOld;
        if(init==0)
        {
            init=1;
            hdc = BeginPaint(hwnd, &ps);
            // Create an off-screen DC for double-buffering
            //hdc = GetDC(hwnd);
            hdcMem = CreateCompatibleDC(hdc);
            hbmMem = CreateCompatibleBitmap(hdc, 1280, 1000);
            EndPaint(hwnd,&ps);
        }

        DISPLOCK();
        hdc = BeginPaint(hwnd, &ps);
        hOld   = SelectObject(hdcMem, hbmMem);

            // Draw into hdcMem here
            //清除背景
            FillRect(hdcMem, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW));

            //画边界

            drawRect(hdcMem,RGB(20, 2, 5),
                     BLOCK_SIZE-3,BLOCK_SIZE-3,
                     mapBasic.n*BLOCK_SIZE+BLOCK_SIZE+3,
                     mapBasic.m*BLOCK_SIZE+BLOCK_SIZE+3);


            //画障碍物
            for(int i=0;i<mapBasic.obst_count;i++)
            {
                drawBox(hdcMem,mapBasic.Xarray[i],mapBasic.Yarray[i]);
            }

            //画蛇
            drawSnake(hdcMem,RGB(10,180,20),snake0);
            drawSnake(hdcMem,RGB(180,10,20),snake1);


            //checkScreenProperty(hdc);

/*
            GetClientRect(hwnd,&lprc);
            DrawText(hdc,"Hello World",-1,&lprc,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
*/
            // Transfer the off-screen DC to the screen
            BitBlt(hdc, 0, 0,
                   ps.rcPaint.right-ps.rcPaint.left,
                   ps.rcPaint.bottom-ps.rcPaint.top,
                   hdcMem, 0, 0, SRCCOPY);

            SelectObject(hdcMem, hOld);

        EndPaint(hwnd,&ps);
        DISPUNLOCK();
        break;
    }
    case WM_KEYDOWN:
        keyboard(wParam);
        break;

    case WM_DESTROY:
        stopRun();
        DeleteObject(hbmMem);
        DeleteDC    (hdcMem);
        PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
        break;
    default:                      /* for messages that we don't deal with */
        return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}



void updateDisp(MapBasic &mb,Snake &snk0,Snake &snk1)
{
    DISPLOCK();
    mapBasic=mb;
    snake0=snk0;
    snake1=snk1;
    thread_comm.refresh=true;
    DISPUNLOCK();
}

void drawRect(HDC hdc,COLORREF color,int x1,int y1,int x2,int y2)
{

    HPEN hPen = CreatePen(PS_SOLID, 4, color);
    HPEN hPenOld = (HPEN)SelectObject(hdc, hPen);

    MoveToEx(hdc, x1, y1, NULL);
    LineTo(hdc, x2, y1);
    LineTo(hdc, x2, y2);
    LineTo(hdc, x1, y2);
    LineTo(hdc, x1, y1);
    SelectObject(hdc, hPenOld);
    DeleteObject(hPen);
}

#include <list>
#define SNAKE_GAP 2
static void drawSnake(HDC hdc,COLORREF color,Snake &snake)
{
    RECT lprc;
    HBRUSH hBrush = CreateSolidBrush(color);
    list<int>::iterator itx,ity;
    int preX=-1,preY=-1;  //为了一条蛇而不是一堆色块
    for(itx=snake.x.begin(),ity=snake.y.begin();
                itx!=snake.x.end(); itx++,ity++)
    {
        SetRect(&lprc,
                *itx*BLOCK_SIZE+SNAKE_GAP,
                *ity*BLOCK_SIZE+SNAKE_GAP,
                *itx*BLOCK_SIZE+BLOCK_SIZE-SNAKE_GAP,
                *ity*BLOCK_SIZE+BLOCK_SIZE-SNAKE_GAP
                );
        FillRect(hdc, &lprc, hBrush);

        if(preX-*itx==1)
        {
            //向左
            SetRect(&lprc,
                *itx*BLOCK_SIZE+BLOCK_SIZE-SNAKE_GAP,
                *ity*BLOCK_SIZE+SNAKE_GAP,
                *itx*BLOCK_SIZE+BLOCK_SIZE+SNAKE_GAP,
                *ity*BLOCK_SIZE+BLOCK_SIZE-SNAKE_GAP
                );
            FillRect(hdc, &lprc, hBrush);
        }else if(preX-*itx==-1)
        {
             //向右
            SetRect(&lprc,
                *itx*BLOCK_SIZE-SNAKE_GAP,
                *ity*BLOCK_SIZE+SNAKE_GAP,
                *itx*BLOCK_SIZE+SNAKE_GAP,
                *ity*BLOCK_SIZE+BLOCK_SIZE-SNAKE_GAP
                );
            FillRect(hdc, &lprc, hBrush);

        }else if(preY-*ity==-1)
        {
            //向下移动
            SetRect(&lprc,
                *itx*BLOCK_SIZE+SNAKE_GAP,
                *ity*BLOCK_SIZE-SNAKE_GAP,
                *itx*BLOCK_SIZE+BLOCK_SIZE-SNAKE_GAP,
                *ity*BLOCK_SIZE+SNAKE_GAP
                );
            FillRect(hdc, &lprc, hBrush);

        }else if(preY-*ity==1)
        {
            //向上移动
            SetRect(&lprc,
                *itx*BLOCK_SIZE+SNAKE_GAP,
                preY*BLOCK_SIZE-SNAKE_GAP,
                *itx*BLOCK_SIZE+BLOCK_SIZE-SNAKE_GAP,
                preY*BLOCK_SIZE+SNAKE_GAP
                );
            FillRect(hdc, &lprc, hBrush);
        }
        preX=*itx;
        preY=*ity;

    }
    DeleteObject(hBrush);

}


static void drawBox(HDC hdc,int x,int y)
{
    RECT lprc;
    HBRUSH hPurpleBrush = CreateSolidBrush(RGB(100, 20, 40));
    for(int i=0;i<mapBasic.obst_count;i++)
    {
        SetRect(&lprc,
                x*BLOCK_SIZE+6,
                y*BLOCK_SIZE+6,
                x*BLOCK_SIZE+BLOCK_SIZE-6,
                y*BLOCK_SIZE+BLOCK_SIZE-6
                );
        FillRect(hdc, &lprc, hPurpleBrush);
    }
    DeleteObject(hPurpleBrush);

    HPEN hBluePen = CreatePen(PS_SOLID, 6, RGB(150, 140, 200));
    HPEN hPen = (HPEN)SelectObject(hdc, hBluePen);

    MoveToEx(hdc, x*BLOCK_SIZE+4, y*BLOCK_SIZE+4, NULL);
    LineTo(hdc, x*BLOCK_SIZE+BLOCK_SIZE-4, y*BLOCK_SIZE+4);
    LineTo(hdc, x*BLOCK_SIZE+BLOCK_SIZE-4, y*BLOCK_SIZE+BLOCK_SIZE-4);
    LineTo(hdc, x*BLOCK_SIZE+4, y*BLOCK_SIZE+BLOCK_SIZE-4);
    LineTo(hdc, x*BLOCK_SIZE+4, y*BLOCK_SIZE+4);

    SelectObject(hdc, hPen);
    DeleteObject(hBluePen);

}


static void keyboard(WPARAM wParam)
{
    switch (wParam)
    {
    case VK_UP:
        keyPressed(3);
        break;

    case VK_DOWN:
        keyPressed(1);
        break;

    case VK_LEFT:
        keyPressed(0);
        break;

    case VK_RIGHT:
        keyPressed(2);
        break;

       case VK_HOME:
        keyPressed(10);
        break;

    case VK_END:
        keyPressed(11);
        break;
    }
}

void DISPLOCK()
{
    EnterCriticalSection(&(thread_comm.displock));
}

void DISPUNLOCK()
{
    LeaveCriticalSection(&(thread_comm.displock));
}

/*************Utility****************/
static void checkScreenProperty(HDC hdc)
{
        HBITMAP memBmp=CreateCompatibleBitmap(hdc,
                        GetSystemMetrics(SM_CXSCREEN),
                        GetSystemMetrics(SM_CYSCREEN));
        BITMAPINFO binfo;
        binfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
        binfo.bmiHeader.biBitCount=0;
        GetDIBits(hdc,memBmp,
                  0,binfo.bmiHeader.biHeight,NULL,
                  (BITMAPINFO*)&binfo,
                  DIB_RGB_COLORS);
        printf("biWidth %ld, biHeight %ld, biBitCount %d\n",
              binfo.bmiHeader.biWidth,
              binfo.bmiHeader.biHeight,
              binfo.bmiHeader.biBitCount);
        DeleteObject(memBmp);

}



