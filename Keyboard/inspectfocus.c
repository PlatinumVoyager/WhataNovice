#include <Windows.h>
#include <stdio.h>

#pragma comment(lib, "user32.lib")

int main(void)
{
    Sleep(5000);

    HWND nWnd = GetForegroundWindow(); // get the handle to the currently active window

    if (nWnd == NULL)
    {
        printf("Failed to get an active handle to the current windowed application.\n");

        return -1;
    }

    printf("GOT ACTIVE HANDLE: SUCCESS\n");

    // obtain window title
    #define BUF_SIZ 100
    char winTitle[BUF_SIZ];

    GetWindowText(nWnd, winTitle, (int) BUF_SIZ);

    printf("ACTIVE TITLE: %s\n", winTitle);

    return 0;
}