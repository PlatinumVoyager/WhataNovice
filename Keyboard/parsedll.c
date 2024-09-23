#include <Windows.h>
#include <DbgHelp.h>
#include <stdio.h>

#pragma comment(lib, "Dbghelp.lib")

#define DLL_TARGET "testdll.dll"


int main(void)
{
    HMODULE hMod = LoadLibrary(DLL_TARGET);

    if (hMod == NULL)
    {
        printf("DLL NON-EXISTENT!\n");
        
        return -1;
    }

    printf("[\033[0;34m*\033[0;m] Successfully loaded target DLL: %s\n\n", DLL_TARGET);

    // Get the NT header of the DLL
    PIMAGE_NT_HEADERS dllNTHeader = ImageNtHeader(hMod);

    if (dllNTHeader == NULL)
    {
        printf("FAILED TO OBTAIN DLL NT HEADERS!\n");

        return -1;
    }

    DWORD dllSize = dllNTHeader->OptionalHeader.SizeOfImage; // Get the size of the DLL
    WORD nSections = dllNTHeader->FileHeader.NumberOfSections; // Get the # of sections in the DLL
    DWORD dllEntryPoint = dllNTHeader->OptionalHeader.AddressOfEntryPoint; // Get the entry point of the DLL

    printf("\t++ Number of sections: %d\n", nSections);
    printf("\t++ Image size: %lu bytes\n", dllSize);
    printf("\t++ DLL Entry Point: 0x%1x | DLL Signature: 0x%1x\n", dllEntryPoint, dllNTHeader->Signature);

    DWORD exSize = 256;
    DWORD imSize = 0;

    printf("\n\t++ Checking for presence of DLL export table\n");

    // Get the export table of the DLL
    PIMAGE_EXPORT_DIRECTORY pExpTable = (PIMAGE_EXPORT_DIRECTORY) ImageDirectoryEntryToData(hMod, TRUE, IMAGE_DIRECTORY_ENTRY_EXPORT, &exSize);

    if (pExpTable == NULL)
    {
        printf("\n\t## DLL export table is non-existent: DLL may not invoke exports\n");
    }

    DWORD nDLL = 0;

    printf("\t++ Checking for presence of DLL import table\n");

    PIMAGE_IMPORT_DESCRIPTOR pImpTable = (PIMAGE_IMPORT_DESCRIPTOR) ImageDirectoryEntryToData(hMod, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &imSize);

    if (pImpTable == NULL)
    {
        printf("NO IMPORT TABLE FOUND!\n");

        return -1;
    }

    printf("\n\t** FOUND DLL IMPORT TABLE!\n");

    PIMAGE_IMPORT_DESCRIPTOR dllDesc = pImpTable;

    do
    {
        printf("%lu\n", dllDesc->OriginalFirstThunk);
        nDLL++;

    } while (dllDesc->OriginalFirstThunk);

    printf("\n\n\t@@ # of Imported DLL\'s: %lu", nDLL);

    FreeLibrary(hMod);

    return 0;
}