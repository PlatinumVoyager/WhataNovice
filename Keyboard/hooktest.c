#include <stdio.h>
#include <Windows.h>

#pragma comment(lib, "user32.lib")

/*
    Test limited feature for actively hooking the end users physical mouse on the current system.
*/

// Mouse hook procedure
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        switch (wParam)
        {
            case WM_LBUTTONDOWN:
            {
                POINT mousePos;
                GetCursorPos(&mousePos);
                printf("Left mouse button clicked at (%d, %d)\n", mousePos.x, mousePos.y);
                break;
            }
            case WM_RBUTTONDOWN:
            {
                POINT mousePos;
                GetCursorPos(&mousePos);
                printf("Right mouse button clicked at (%d, %d)\n", mousePos.x, mousePos.y);
                break;
            }
            case WM_MBUTTONDOWN:
            {
                POINT mousePos;
                GetCursorPos(&mousePos);
                printf("Middle mouse button clicked at (%d, %d)\n", mousePos.x, mousePos.y);
                break;
            }
            default:
                break;
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main()
{
    // Install mouse hook
    HHOOK mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, NULL, 0);
    if (mouseHook == NULL)
    {
        printf("Error installing mouse hook.\n");
        return 1;
    }

    printf("Mouse hook installed. Press any key to exit.\n");

    while (1)
    {
        // Check for key presses
        for (int i = 0; i < 256; i++)
        {
            SHORT state = GetAsyncKeyState(i);
            if (state & 0x8000)
            {
                // Check if the Shift key is pressed
                SHORT shift_state = GetKeyState(VK_SHIFT);
                BOOL is_shift_pressed = (shift_state & 0x8000) != 0;

                // Convert the key code to a character
                BYTE keyboard_state[256] = {0};
                keyboard_state[VK_SHIFT] = is_shift_pressed ? 0x80 : 0;
                WORD unicode_char;
                ToUnicode(i, 0, keyboard_state, &unicode_char, 1, 0);

                // Print the character
                printf("%c", (char)unicode_char);
            }
        }

        // Handle Windows messages to keep the hook running
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Exit loop if any key is pressed
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
        {
            break;
        }
    }

    // Clean up hook
    UnhookWindowsHookEx(mouseHook);

    return 0;
}