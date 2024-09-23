#include <WinSock2.h>
#include <Windows.h>
#include <iphlpapi.h>
#include <wininet.h>
#include <stdio.h>

// PROGRAM RETURNS NULL, FUNCTION CALL MIGHT BE DEPRECATED


#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "wininet.lib")


int main(void)
{
    char narrowUrl[256];
    LPCWSTR url = L"www.youtube.com";

    int newChar = WideCharToMultiByte(CP_ACP, 0, url, -1, narrowUrl, 256, NULL, NULL);
    if (newChar == 0)
    {
        fprintf(stderr, "[!] Failed to convert wide charset to narrow charset. Error during conversion. Failed!\n");

        return -1;
    }

    BOOL hasConnection = InternetCheckConnectionA(narrowUrl, FLAG_ICC_FORCE_CONNECTION, 0);
    if (hasConnection == TRUE)
    {
        printf("[+] Connection is active...\n");
    }
    else
    {
        DWORD ipError = GetLastError();
        LPSTR errorMsg = NULL;

        if (ipError == ERROR_NOT_CONNECTED)
        {
            fprintf(stderr, "[!] Socket database might be unconditionally offline...\n");
        }

        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, ipError,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&errorMsg, 0, NULL);

        fprintf(stderr, "Error: %s\n", errorMsg);
        LocalFree(errorMsg);

        return -1;
    }

    getchar();

    return 0;
}