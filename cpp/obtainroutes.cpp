#include <WinSock2.h>
#include <Windows.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "Ws2_32.lib")

int main(void)
{
    PMIB_IPFORWARDTABLE wcIpFwdTable;

    DWORD wcSize = 0;
    DWORD status = GetIpForwardTable(NULL, &wcSize, FALSE); // FALSE = ipv4

    if (status == ERROR_INSUFFICIENT_BUFFER)
    {
        printf("[+] Allocating memory for IP FORWARD lookup table...\n");

        if ((wcIpFwdTable = (PMIB_IPFORWARDTABLE) malloc(wcSize)) == NULL)
        {
            fprintf(stderr, "Failed to allocate buffer #2 for IP forward table!\n");
            free(wcIpFwdTable);

            return -1;
        }
        
        printf("[*] Memory for IP lookup table allocated successfully...\n");

        status = GetIpForwardTable(wcIpFwdTable, &wcSize, FALSE);
        if (status == NO_ERROR)
        {
            printf("\nDestination\tMask\t\tGateway\tInterface\n\n");

            for (DWORD i = 0; i < (int) wcIpFwdTable->dwNumEntries; i++)
            {
                in_addr dstAddr, mask, gateway;

                dstAddr.S_un.S_addr = wcIpFwdTable->table[i].dwForwardDest;
                mask.S_un.S_addr = wcIpFwdTable->table[i].dwForwardMask;
                gateway.S_un.S_addr = wcIpFwdTable->table[i].dwForwardNextHop;

                printf("%s\t%s\t\t%s\t%d\n", inet_ntoa(dstAddr), inet_ntoa(mask), inet_ntoa(gateway), wcIpFwdTable->table[i].dwForwardIfIndex);
            }
        }
        else
        {
            fprintf(stderr, "Failed to obtain routing data! Exiting...\n");
            free(wcIpFwdTable);

            return -1;
        }
    }

    free(wcIpFwdTable);

    printf("\n[*] Memory allocation for IP forward table freed. Press <ENTER> to exit...\n");
    getchar();

    return 0;
}