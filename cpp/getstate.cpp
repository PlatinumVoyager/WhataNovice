#include <Windows.h>
#include <wininet.h>
#include <stdio.h>

#pragma comment(lib, "wininet.lib")

int main(void)
{
    char *state;
    DWORD flags;

    // until a method to use classic switch statement is resolved
    if (InternetGetConnectedState(&flags, 0))
    {
        if (flags &0x01)
        {
            state = "MODEM";
        }
        else if (flags &0x02)
        {
            state = "ETHERNET";
        }
        else if (flags &0x04)
        {
            state = "PROXY";
        }
        else
        {
            state = "UNKNOWN";
        }

        printf("[+] Connection is active: %s\n", state);
    }
    else
    {
        printf("[!] Connection has been lost!\n");
    }

    getchar();

    return 0;
}