/*
    Copyright to no one. Free to public. 
    2021
*/

#include <WinSock2.h>
#include <Windows.h>
#include <iphlpapi.h>
#include <Psapi.h>
// #include <mstcpip.h>
#include <stdio.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "Ws2_32.lib")

#define INET_ADDRSTRLEN 16
#define PROCESS_BUFFER_SIZE 1024

void wcGetBuildVersion(void);
void wcGetProcessInformation(void);

char *tdGreenOpen = "\033[0;32m";
char *tdBlueOpen = "\033[0;34m";
char *tdCloseEsc = "\033[0;m";

int main(void)
{
    wcGetBuildVersion();

    PMIB_TCPTABLE2 tcpTable;
    ULONG uSize = 0;

    // call GetTcpTable2 to get the required size of the table
    if (GetTcpTable2(NULL, &uSize, TRUE) == ERROR_INSUFFICIENT_BUFFER)
    {
        tcpTable = (PMIB_TCPTABLE2) malloc(uSize);
        
        if (tcpTable == NULL)
        {
            fprintf(stderr, "Failed to allocate buffer big enough to hold the TCP_TABLE.\n");

            return -1;
        }
        else
        {
            printf("%s[+]%s Allocated buffer (%zd bytes) large enough to hold a future data store.\n", tdGreenOpen, tdCloseEsc,
                                                                                                       sizeof(tcpTable));
        }
    }

    char *dwState;

    // retrieve the table now
    if (GetTcpTable2(tcpTable, &uSize, TRUE) == NO_ERROR)
    {
        HANDLE mutex = CreateMutex(NULL, FALSE, NULL);

        if (mutex == NULL)
        {
            fprintf(stderr, "Failed to create a mutex for thread safe parsing of IP addresses!\n");

            return -1;
        }

        printf("%s[+]%s Thread safe Mutex created..initializing...\n\n", tdGreenOpen, tdCloseEsc);

        for (DWORD i = 0; i < (int) tcpTable->dwNumEntries; i++)
        {
            MIB_TCPROW2 tcpRow = tcpTable->table[i];

            DWORD pState = tcpRow.dwState;
            DWORD procID = tcpRow.dwOwningPid;
            
            DWORD localPort = tcpRow.dwLocalPort;
            DWORD remotePort = tcpRow.dwRemotePort;

            if (pState &MIB_TCP_STATE_CLOSED)
            {
                dwState = "CLOSED";
            }
            else if (pState &MIB_TCP_STATE_LISTEN)
            {
                dwState = "LISTENING";
            }
            else if (pState &MIB_TCP_STATE_ESTAB)
            {
                dwState = "ESTABLISHED";
            }
            else
            {
                dwState = "UNKNOWN";
            }

            char *tcpLocalAddr;
            char *tcpRemoteAddr;

            // acquire the mutex before calling inet_ntoa
            WaitForSingleObject(mutex, INFINITE);

            tcpLocalAddr = inet_ntoa(*(struct in_addr *)&tcpRow.dwLocalAddr);
            tcpRemoteAddr = inet_ntoa(*(struct in_addr *)&tcpRow.dwRemoteAddr);

            // release the mutex
            if (!ReleaseMutex(mutex))
            {
                fprintf(stderr, "Failed to release the mutex! Shutting down!\n");

                CloseHandle(mutex);
                return -1;
            }
            else
            {
                printf("REMOTE: %s:%d, LOCAL: %s:%d -> PID: %d (%s)\n", tcpRemoteAddr, ntohs(remotePort), 
                                                                        tcpLocalAddr, ntohs(localPort), 
                                                                        procID, dwState
                );
            }
        }

        CloseHandle(mutex);
    }
    else
    {
        fprintf(stderr, "Failed to obtain the TCP_TABLE after allocated buffer (malloc)\n");

        return -1;
    }

    printf("\n%s[*]%s Calling funtion to obtain verbose process information. Press <ENTER> to continue...\n", tdBlueOpen, tdCloseEsc);
    getchar();

    wcGetProcessInformation();
    getchar();

    free(tcpTable);

    return 0;
}


void wcGetBuildVersion(void)
{
    OSVERSIONINFOEX opVersion;

    ZeroMemory(&opVersion, sizeof(OSVERSIONINFOEX));
    opVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if (GetVersionEx((OSVERSIONINFO *)&opVersion))
    {
        printf("%s[*]%s Build: %d.%d.%d\n", tdBlueOpen, tdCloseEsc, opVersion.dwMajorVersion, opVersion.dwMinorVersion, opVersion.dwBuildNumber);
    }
}


void wcGetProcessInformation(void)
{
    DWORD procID[PROCESS_BUFFER_SIZE], bytesNeeded, currProcess;

    if (EnumProcesses(procID, sizeof(procID), &bytesNeeded))
    {
        currProcess = bytesNeeded / sizeof(DWORD); // DWORD = 32 bits

        for (DWORD i = 0; i < currProcess; i++)
        {
            HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procID[i]);

            if (hProc)
            {
                TCHAR procName[MAX_PATH] = TEXT("UNKNOWN");
                DWORD size = sizeof(procName) / sizeof(*procName);

                QueryFullProcessImageName(hProc, 0, procName, &size);

                PROCESS_MEMORY_COUNTERS_EX pmCounter;

                if (GetProcessMemoryInfo(hProc, (PROCESS_MEMORY_COUNTERS *)&pmCounter, sizeof(pmCounter)))
                {
                    printf("FILE PATH: \"%s\" (PID=%d), MEM: \033[0;32m%zd\033[0;m MB\n", procName, procID[i], pmCounter.PrivateUsage / PROCESS_BUFFER_SIZE / PROCESS_BUFFER_SIZE);
                }
            }

            CloseHandle(hProc);
        }
    }
}