#include <stdio.h>
#include <Windows.h>
#include <LM.h>
#include <errno.h>

/*
    this program will utilize the windows api to obtain a listing to the current
    user accounts on the local system
*/

#pragma comment(lib, "netapi32.lib")

#define BLOCK_EXPRESSION "==============================================="

int main(void)
{
    DWORD level = 1;
    DWORD entries_read = 0;
    DWORD total_entries = 0;
    DWORD resume_handle = 0;
    DWORD prefmaxlen = MAX_PREFERRED_LENGTH;

    NET_API_STATUS napi_status;
    LPUSER_INFO_1 user_info_buff = NULL;

    napi_status = NetUserEnum(NULL, level, FILTER_NORMAL_ACCOUNT, (LPBYTE *)&user_info_buff, &prefmaxlen, 
                                                        &entries_read, &total_entries, &resume_handle);

    if (napi_status == NERR_Success)
    {
        fprintf(stderr, "USER ENTRIES ON SYSTEM: %d ..Press <ENTER> to obtain exhaustive listing...\n", total_entries);

        getchar();
        puts(BLOCK_EXPRESSION);

        for (DWORD i = 0; i < entries_read; i++)
        {
            printf("USER: %ls:\"%ls\"\n", user_info_buff[i].usri1_name, user_info_buff[i].usri1_home_dir);
        }

        puts(BLOCK_EXPRESSION);
    }
    else
    {
        fprintf(stderr, "ERROR: %s (%d)\n", strerror(napi_status), napi_status);
    }

    NetApiBufferFree(user_info_buff);
    getchar();

    return 0;
}