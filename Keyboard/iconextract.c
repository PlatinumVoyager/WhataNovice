#include <Windows.h>
#include <Psapi.h>
#include <stdio.h>

#pragma comment(lib, "user32.lib")

#define PROCESS_BUFFER_SIZE 1024
#define EXIT_PROCESS_FAIL exit(-1)

// this file should be able to get the full path to an executable and extract the icon image from it
TCHAR *getProcessInformationExe(char *path);


int main(int argc, char *argv[])
{
    HICON exeIcon;
    TCHAR *exePath;

    LPCTSTR lexeFileName;

    char *path = NULL;

    if (argc == 2)
    {
        // user defined exe path
        path = argv[1];
        exePath = path;

        printf("PATH: %s\n", exePath);
    }
    else
    {
        printf("[\033[0;34m*\033[0;m] No active operator SET process, using defaults\n");

        exePath = getProcessInformationExe(NULL);

        printf("DEFAULT: \"%s\"\n", exePath);

        // if we really need to get the pid and size 
    }

    // HMODULE mLoadLibrary = LoadLibraryEx()

    return 0;
}

TCHAR *getProcessInformationExe(char *path)
{
    TCHAR procName[MAX_PATH] = TEXT("UNKNOWN");
    TCHAR *returnName;

    DWORD procID[PROCESS_BUFFER_SIZE], bytesNeeded, currProcess;

    if (EnumProcesses(procID, sizeof(procID), &bytesNeeded))
    {
        currProcess = bytesNeeded / sizeof(DWORD); // DWORD = 32 bits

        for (DWORD i = 0; i < currProcess; i++)
        {
            HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procID[i]);

            if (hProc) // for each concurrent processes
            {
                DWORD procSize = sizeof(procName) / sizeof(*procName);

                if (!QueryFullProcessImageName(hProc, 0, procName, &procSize))
                {
                    printf("FAILED TO QUERY PROCESS IMAGE!\n");
                    EXIT_PROCESS_FAIL;
                }

                // the buffer being allocated is not only be allocated for the length of procName
                printf("\n++ Buffer of %zd bytes allocated @%p\n\n", sizeof(returnName), &returnName);

                // do we need to allocate dynamic memory on the heap to pass string returned from procName back to caller?
                returnName = (TCHAR *)malloc(procSize * sizeof(TCHAR));

                if (returnName == NULL)
                {
                    printf("FAILED TO ALLOCATE BUFFER FOR PROCESS!\n");
                    EXIT_PROCESS_FAIL;
                }

                if (procSize > 0) // better than (strlen(procName) > 0), as it allocated extra memory
                {
                    strcpy(returnName, procName);

                    // printf("%d\n", wcslen(returnName));

                    if (strlen(returnName) == 0)
                    {
                        printf("FAILED TO COPY TARGET PROCESS NAME TO ALLOCATED BUFFER!\n");
                        EXIT_PROCESS_FAIL;
                    }

                    // need to return procName to the caller as a string
                    // on windows you can assume 2 byte characters for unicode support instead of std 1 byte char
                    printf("[\033[0;32m+\033[0;m] Primary process string: %s (total=%zd, actual=%d, pid=%d)\n", procName, strlen(procName), procSize *= 2, procID[i]); // procSize *= 2;

                    // maybe write data to object in memory and extract area from main, possibly have this function 
                    // return the address of the object in memory

                    return returnName;
                }
            }

            CloseHandle(hProc);
        }
    }

    printf("REACHED BOTTOM OF PROCESS FUNCTION EXECUTION!\n");

    return NULL;
}