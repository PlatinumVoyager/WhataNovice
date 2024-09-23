#include <Windows.h>
#include <stdio.h>

/*
    mousestate.c - obtains a valid hook to the windows mouse events
*/

#pragma comment(lib, "user32.lib")

POINT GetMouseCoordinates(void);
LRESULT CALLBACK MouseProc(int mCode, WPARAM mwParam, LPARAM mlParam);

int main(void)
{
    HHOOK mHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, NULL, 0);

    if (mHook == NULL)
    {
        printf("FAILED TO OBTAIN HOOK ON MOUSE!\n");

        return -1;
    }

    printf("[\033[0;34m*\033[0;m] Mouse Hook Status: \033[0;32mESTABLISHED\033[0;m\n\n");

    MSG mMsg = {0};

    while (GetMessage(&mMsg, NULL, 0, 0))
    {
        TranslateMessage(&mMsg);
        DispatchMessage(&mMsg);
    }

    UnhookWindowsHookEx(mHook);
}

LRESULT CALLBACK MouseProc(int mCode, WPARAM mwParam, LPARAM mlParam)
{
    if (mCode == HC_ACTION)
    {
        switch (mwParam)
        {
            case WM_LBUTTONDOWN:
                POINT WM_LBTN = GetMouseCoordinates();
                printf("Left mouse button clicked (%d, %d)\n", WM_LBTN.x, WM_LBTN.y);
                
                break;

            case WM_MBUTTONDOWN:
                POINT WM_MBTN = GetMouseCoordinates();
                printf("Middle mouse button clicked (%d, %d)\n", WM_MBTN.x, WM_MBTN.y);
                
                break;

            case WM_RBUTTONDOWN:
                POINT WM_RBTN = GetMouseCoordinates();
                printf("Right mouse button clicked (%d, %d)\n", WM_RBTN.x, WM_RBTN.y);
                
                break;

            default:
                break;
        }

    }

    return CallNextHookEx(NULL, mCode, mwParam, mlParam);
}

POINT GetMouseCoordinates(void)
{
    // this function returns the current mouse position when invoked by the caller
    POINT mousePnt;
    GetCursorPos(&mousePnt);

    return mousePnt;
}