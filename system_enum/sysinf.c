#include <windows.h>
#include <sysinfoapi.h>
#include <TlHelp32.h>
#include <libloaderapi.h>

/* process output stream definitions */
#include <stdio.h>

#include <enclaveapi.h>

/* include kernel32 DLL without explicitly linking it */
#pragma comment(lib, "Kernel32.lib")

// #define _WIN32_WINNT 0x0501

/*
    @param[in] - None

    Primary structure that will hold
    system header information. SHI is a custom
    defined data glob that contains system related
    information data types.
*/
typedef struct _SYSINF_MAIN
{
    DWORD       systemOEMID;
    WORD        systemProcArchitecture;
    DWORD       systemPageSz;

    LPVOID      minimumAppAddress;
    LPVOID      maximumAppAddress;

    DWORD_PTR   activeProcMask;
    DWORD       systemProcCount;
    DWORD       systemProcType;

    DWORD       allocGranularity;
    WORD        systemProcLevel;
    WORD        systemProcRevision;

} SYSINF_MAIN, *PSYSINF_MAIN;

/*
    @param[in] - None

    Enum structure that contains the different type of values for
    processor architecture types against different instruction sets.
*/
typedef enum processorTypes
{
    AMD64 =     PROCESSOR_ARCHITECTURE_AMD64,
    ARM =       PROCESSOR_ARCHITECTURE_ARM,
    ARM64 =     PROCESSOR_ARCHITECTURE_ARM64,
    IA64 =      PROCESSOR_ARCHITECTURE_IA64,
    MIPS =      PROCESSOR_ARCHITECTURE_MIPS,
    POWERPC =   PROCESSOR_ARCHITECTURE_PPC,
    X86 =       PROCESSOR_ARCHITECTURE_INTEL,
    UNKNOWN =   PROCESSOR_ARCHITECTURE_UNKNOWN

} PROCESSOR_TYPES;

HMODULE getKernel32DLLHandle(void);
DWORD processSystemHeader(SYSTEM_INFO sysInfo);
LPWSTR returnMsgBuffer(DWORD errorCode);

DWORD getSystemFirmwareType(void);
DWORD getSystemVersionInfo(void);

SYSTEM_INFO sysInfo = {0};
SYSINF_MAIN sysinfHdr = {0};

/* magic pointer to system information structure */
SYSINF_MAIN *sysinfHdrMain = &sysinfHdr;

/* function pointer prototype definition */
typedef BOOL(WINAPI *GetPhysicallyInstalledSystemMemoryType)(PULONGLONG);

/* obtain a handle to the currently loaded kernel32 DLL in the current process */
HMODULE getKernel32DLLHandle(void)
{
    HMODULE hKernel32DLL = GetModuleHandleA("kernel32.dll");

    if (hKernel32DLL == NULL)
    {
        fwprintf(stderr, L"## ERR::STATUS => failed to get a handle to the \"kernel32.dll\" target...\n");
        exit(-1);
    }

    return hKernel32DLL;
}


/* short function: firmare type enumeration */
DWORD getSystemFirmwareType(void)
{
    PFIRMWARE_TYPE fwType = {0};
    DWORD firmwalk = GetFirmwareType(&fwType);

    if (firmwalk == 0)
    {
        DWORD error = GetLastError();
        fwprintf(stderr, L"## ERR::STATUS => failed to call \"GetFirmwareType()\" to obtain FWT!\nERROR: %s\n", returnMsgBuffer(error));

        return (DWORD) -1;
    }

    switch (firmwalk)
    {
        /* The firmware type is unknown. */
        case FirmwareTypeUnknown:
        {
            printf("FIRMWARE TYPE: ???\n");

            break;
        }

        /* The computer booted in legacy BIOS mode. */
        case FirmwareTypeBios:
        {
            printf("FIRMWARE TYPE: BIOS LEGACY MODE\n");

            break;
        }

        /* The computer booted in UEFI mode. */
        case FirmwareTypeUefi:
        {
            printf("FIRMWARE TYPE: UEFI MODE\n");

            break;
        }

        /* Not implemented. */
        case FirmwareTypeMax:
        {
            printf("FIRMWARE TYPE: NOT IMPLEMENTED\n");

            break;
        }
    }

    /* generic success */
    return (DWORD) 0;
}


DWORD getSystemVersionInfo(void)
{
    BOOL versionRet;

    /* primary OS structure */
    OSVERSIONINFO xtraOSInfo;

    /* targeted structure to use bit comparison operations */
    OSVERSIONINFOEX dummyOSInfo;

    /* zero out structure field members */
    memset((&xtraOSInfo), 0x0, sizeof(OSVERSIONINFO));
    memset((&dummyOSInfo), 0x0, sizeof(OSVERSIONINFOEX));

    /* populate version size information */
    xtraOSInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    /* sizeof OSVERSIONINFOEX (allocate enough space for ALL structure members) */
    dummyOSInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    /* check version information generically (GetVersionExA()) */
    /* statically perform verification checks against the targeted OS (VerifyVersionInfoA()) */

    DWORD bitMask = 0;
    DWORDLONG conditional = 0;

    dummyOSInfo.dwMajorVersion = 6;
    dummyOSInfo.dwMinorVersion = 2;

    VER_SET_CONDITION(bitMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
    VER_SET_CONDITION(bitMask, VER_MINORVERSION, VER_GREATER_EQUAL);

    conditional = VER_MAJORVERSION | VER_MINORVERSION;

    /* do we fail? */
    if ((versionRet = GetVersionExA((OSVERSIONINFO*)&dummyOSInfo)) == (VARIANT_BOOL)0)
    {
        fwprintf(stderr, L"## ERR::STATUS => failed to call \"GetVersionExA()\" to get system OS structure information!\n");

        return (DWORD) -1;
    }
    
    switch (versionRet)
    {
        case TRUE:
        {
            wprintf(L"MAJOR: %d, MINOR: %d\n", dummyOSInfo.dwMajorVersion, dummyOSInfo.dwMinorVersion);
            
            break;
        }

        default:
            break;
    }

    /* copy structure contents (targeted) */
    xtraOSInfo.dwMajorVersion = (DWORD)dummyOSInfo.dwMajorVersion;
    xtraOSInfo.dwMinorVersion = (DWORD)dummyOSInfo.dwMinorVersion;

    if (VerifyVersionInfoA((OSVERSIONINFOEXA*)&dummyOSInfo, conditional, conditional))
    {
        /* last check if application is manifested for windows 10 (TARGET) */
        if ((xtraOSInfo.dwMajorVersion <= (DWORD)6) && (xtraOSInfo.dwMinorVersion <= (DWORD)2))
        {
            fwprintf(stderr, L"?? INF::STATUS => OS is not manifested for windows 10 (MAJOR.MINOR=%lu.%lu)\n",
                xtraOSInfo.dwMajorVersion,
                xtraOSInfo.dwMinorVersion
            );

            // if really need be, DLL inject (or statically analyze file) into a process that was recently updated
            // and obtain the PE32+ (64 bit) Optional-Header MajorOperatingSystemVersion, MinorOperatingSystemVersion
            // information to get the system version number instead (or interop C++ wrapper for helper functions)
        }
        else
        {
            /* XML manifest was found within the current process (VAS (Virtual Address Space)) */
            printf("OS VERSION: %lu.%lu (MAJOR.MINOR)\n",
                xtraOSInfo.dwMajorVersion,
                xtraOSInfo.dwMinorVersion
            );
        }   
    }
    else 
    {
        /* defined? do nothing */
        ;;
    }

    /* generic success */
    return (DWORD) 0;
}


int main(void)
{
    /* system return code when processing system header information */
    DWORD sysReturn = -1;

    /* system processor flags */
    DWORD procFlags = 0;

    GetSystemInfo(&sysInfo);

    if ((sysReturn = processSystemHeader(sysInfo)) != 0)
    {
        fwprintf(stderr, L"[ ! ] NOP (No Operation) => failed to process system header information.\n");

        return -1;
    }

    printf("OEM ID: #%d\nARCH ID: %lu\n", 
            *(&sysinfHdr.systemOEMID),
            *(&sysinfHdr.systemProcType)
    );

    getSystemFirmwareType();
    getSystemVersionInfo();

    ULONGLONG totalMem = 0;
    HMODULE hKernel32DLL = getKernel32DLLHandle();

    /* obtain a function pointer to the targeted function from the DLL */
    GetPhysicallyInstalledSystemMemoryType sysMemPtr = (GetPhysicallyInstalledSystemMemoryType) GetProcAddress(hKernel32DLL, "GetPhysicallyInstalledSystemMemory");

    if (sysMemPtr == NULL)
    {
        DWORD error = GetLastError();

        fwprintf(stderr, 
            L"## ERR::STATUS => failed to call \"GetProcAddress()\" to get DLL address for targeted function call!\n"
            L"ERR => %s\n", returnMsgBuffer(error)
        );
        
        return -1;
    }

    printf("MEM_ADDR (GetPhysicallyInstalledSystemMemory) = 0x%p\n", sysMemPtr);

    if (sysMemPtr(&totalMem))
    {
        printf("RAM: %llu GB\n", ((totalMem / 1024) / 1024));
    }

    FreeLibrary(hKernel32DLL);

    /* lockdown operations */
    // {
    //     #include "WinUser.h"
    //     #pragma comment(lib, "user32.lib")

    //     if (!LockWorkStation())
    //     {
    //         DWORD error = GetLastError();

    //         printf("FAILED TO LOCK THE CURRENT DEVICE!\nERR: %s\n", returnMsgBuffer(error));

    //         return -1;
    //     }
    // }

    return 0;
}


DWORD processSystemHeader(SYSTEM_INFO sysInfo)
{
    sysinfHdrMain->systemOEMID = sysInfo.dwOemId + 1000;

    /* setup processor type definitions */
    PROCESSOR_TYPES processorType = sysInfo.wProcessorArchitecture;

    switch (processorType)
    {
        case AMD64:
        {
            printf("PROCTYPE: AMD\n");

            /* verify 64 bit check with IsWoW64() */
            sysinfHdrMain->systemProcType = PROCESSOR_ARCHITECTURE_AMD64;

            break;
        }

        case X86:
        {
            printf("PROCTYPE: INTEL\n");

            /* intel based processor, 32 bit (i386) */
            sysinfHdrMain->systemProcType = PROCESSOR_ARCHITECTURE_INTEL;

            break;
        }

        case ARM:
        case ARM64:
        {
            /* mobile devices, currently not supported */
            ;;
        }

        case IA64:
        {
            /* Intel Itanium-based 64 bit*/
            ;;
        }

        case MIPS:
        {
            /* embedded device, router, etc. */
            ;;
        }

        case POWERPC:
        {
            /* gaming desktop, workstation, etc. */
            ;;
        }

        case UNKNOWN:
        {
            /* system type not tracked, no need to care for it */
            printf("????\n");

            break;
        }

        default:
        {
            printf("????\n");

            break;
        }
    }


    /* return generic success */
    return (DWORD) 0;
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