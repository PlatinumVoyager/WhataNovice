#include <Windows.h>
#include <wingdi.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#define MESSAGE_STRING "C8H10N4O2 (CAFFEINE) LOADER"
#define PIXEL_SIZE_OFFSET 60 // pixel offset is dependant upon the font size, style, and other features/factors, etc

// LPARAM - Long parameter (32 bits (DWORD = unsigned long)), WPARAM - Word parameter (16 bits (WORD = unsigned short))
LRESULT CALLBACK Win32Procedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam); // function prototype for windows api
/*
    HINSTANCE hInstance - instance handle of the application
    HINSTANCE hPrevInstance - unused always NULL
    LPSTR lpCmdLine - command line arguments passed to the application
    int nCmdShow - specifies how the application window is to be displayed
*/

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdLine, int nCmdShow)
{
    // WNDCLASS - is a windows class structure that defines the attributes of a window class
    WNDCLASS windowClass = {0};

    windowClass.lpfnWndProc = Win32Procedure; // defines the custom window procedure (pointer)
    windowClass.hInstance = hInstance; // a handle to the instance that contains the window procedure for the class
    windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW); // a handle to the mouse cursor when positioned over the main window
    windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // tells the system to use the default system color for the window background
    windowClass.lpszClassName = "TESTWINDOWCLASS\0"; // name of the windows class, must be NULL terminated "\0"

    RegisterClassW(&windowClass);

    // HWND = handle to a window
    // a handle is a unique identifier that is assigned to every window
    // a handle allows the operating system to manage and share the window
    // with other applications on the system
    // WS in WS_* = Window Style

    /*
        HWND hWnd = CreateWindow(
            szWindowClass,                      // window class name
            szTitle,                            // window title
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,   // window style
            CW_USEDEFAULT, CW_USEDEFAULT,       // x, y position
            CW_USEDEFAULT, CW_USEDEFAULT,       // width, height
            NULL,                               // parent window
            NULL,                               // menu handle
            hInstance,                          // application instance handle
            NULL                                // additional application data
        );
    */

    HWND hwnd = CreateWindowW(
        "TESTWINDOWCLASS\0", 
        "Generic DLL Memory mapper",
        // WS_OVERLAPPEDWINDOW means that the window should have a common window style applied, 
        // such as a title bar, a border, and a system menu along with a sizing border, etc.
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
        CW_USEDEFAULT, CW_USEDEFAULT, // CW = Create Window
        600, // width of the window
        300, // height of the window
        NULL, 
        NULL, 
        hInstance, 
        NULL
    );

    // HDC = handle to device context
    // a handle is a value that identifies an object in the windows operating system
    HDC hdc = GetDC(hwnd);

    TextOutW(hdc, 10, 10, MESSAGE_STRING, 8); // 8 = "STATUS: " (length of uncolored text)

    // Initialize text color
    SetTextColor(hdc, RGB(0, 255, 0)); // GREEN
    TextOutW(hdc, 10 + PIXEL_SIZE_OFFSET, 10, MESSAGE_STRING + 8, 22);

    ReleaseDC(hwnd, hdc);

    // MSG - a structure that contains the information about the next message in the que (within the context of the current
    // window application) to be processed by the window procedure
    MSG msg = {0}; // {0} = set all MSG struct members to 0

    /*
        The window (GUI) will recieve messages from the operating system in the form of user input, window resizing, etc.
        All of these messages are stored in a message que, the application has the responsibility to retrieve these messages
        from the que and process them

        The 'GetMessage()' function retrieves the next message from the que, and places it into a 'MSG' structure.
        This MSG structure contains information about the message such as: 
                1. The message type
                2. The window that the message is for
                3. Any additional data associated with the message

        The below "while (){}" loop construct is continuously used to get messages from the que and process them, 
        if no message is present 'GetMessage()' blocks until a message becomes available
    */

    while (GetMessageW(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return msg.wParam;
}

// The operating system will utilize this callback function to process
// message events sent to the windowed application (i.e current graphical process)
// LRESULT CALLBACk indicates that this is a window procedure that processes messages sent to the window
// and returns an appropriate response code

// LPARAM, and WPARAM contain information related to messages sent to the application (ex: mouse coordinates, mouse side clicked, etc)
// as to give further context to the custom window procedure callback function when a system message is post-processed
LRESULT CALLBACK Win32Procedure(HWND hwnd, UINT msg, WPARAM WParam, LPARAM LParam)
{
    switch (msg)
    {
        case 0x0002: // WM_DESTROY (WM = Window Message)
        {
            // Posts a "quit" message to the message que, which signals the end of the main application loop
            PostQuitMessage(0);
            break;
        }

        // Handles any other events (messages) that are not setup to be properly processed by the calling application
        // by passing said unhandled events to the default windows procedure
        default:
        {
            return DefWindowProcW(hwnd, msg, WParam, LParam);
        }
    }

    return 0;
}