// With l<3ve from CHAT-GPT3

// The next step is to transform these simple CLI's into simple GUI's

#include <Windows.h>
#include <stdio.h>
#include <TlHelp32.h>
#include <string.h>
#include <wchar.h>

#define EXW_SHUTDOWN 0x00000001
#define EXW_FORCE 0x00000004

int MODE_BIT = 0;

PROCESSENTRY32 ObtainProcessInfo(DWORD pid, int mbit);

int main(int argc, char *argv[])
{
    DWORD pid = NULL;

    if (!argv[1])
        // no target pid set: MODE_BIT = 0;
        MODE_BIT = 0;
    else
    {
        MODE_BIT = 1;
        int argvPid = atoi(argv[1]);

        pid = (DWORD) argvPid;
    }

    PROCESSENTRY32 pe32 = ObtainProcessInfo(pid, MODE_BIT);

    wprintf(L"TARGET: \"%hs\" (PPID=%lu, PID=%lu)\n", pe32.szExeFile, pe32.th32ParentProcessID, pid);

    return 0;
}

PROCESSENTRY32 ObtainProcessInfo(DWORD pid, int mbit)
{
    // PROCESSENTRY32 is a structure that will contain the information related to the target process
    // {0} is used to initialize every member of the PROCESSENTRY32 structure to 0, 
    // if it was 0 instead, only the first member of the structure would be set to 0
    PROCESSENTRY32 procEntry32 = { 0 };

    // default to normal size of PROCESSENTRY32 structure
    procEntry32.dwSize = sizeof(PROCESSENTRY32);

    // TH32CS_SNAPPROCESS Includes all processes in the system in the snapshot
    HANDLE h32Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    
    if (h32Snapshot != INVALID_HANDLE_VALUE)
    {
        if (Process32First(h32Snapshot, &procEntry32))
        {
            if (MODE_BIT == 0)
            {
                if (procEntry32.th32ProcessID)
                {
                    CloseHandle(h32Snapshot);

                    return procEntry32;
                }
            }
            else if (MODE_BIT == 1)
            {
                do
                {
                    if (procEntry32.th32ProcessID == pid)
                    {
                        printf("[*] Got snapshot data from process entry listing. Obtaining results...\n");
                        CloseHandle(h32Snapshot);

                        return procEntry32;
                    }
                } while (Process32Next(h32Snapshot, &procEntry32));
            }
        }

        CloseHandle(h32Snapshot);
    }

    procEntry32.th32ProcessID = 0; // process not found (this will just return the root PID)

    return procEntry32;
}