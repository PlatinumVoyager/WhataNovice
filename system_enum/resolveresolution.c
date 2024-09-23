#include <Windows.h>
#include <stdio.h>

#pragma comment(lib, "winmm.lib");

int queryDeviceResolutionMinMax(void);
LPWSTR returnMsgBuffer(DWORD errorCode);


int main(void)
{
    int rc = queryDeviceResolutionMinMax();

    __asm {
        
    }

    switch (rc)
    {
        case 0:
            break;

        /* force exit through the main function */
        case -1:
            return -1; // syn exit(-1);
    }

    return 0;
}


int queryDeviceResolutionMinMax(void)
{
    UINT cbtc;
    TIMECAPS tc; // timeCap pointer

    MMRESULT ret;

    if ((ret = timeGetDevCaps((LPTIMECAPS)&tc, sizeof(TIMECAPS))) == MMSYSERR_NOERROR)
    {
        /* success */ 
        wprintf(L"CPU RES => (MIN): %u, (MAX): %u\n", tc.wPeriodMin, tc.wPeriodMax);
    }
    else
    {
        /* generic error handling via win32 API */
        switch (ret)
        {
            case MMSYSERR_ERROR:
            {   
                DWORD error = GetLastError();

                fwprintf(stderr, L"ERR => %s\n", returnMsgBuffer(error));

                return -1;
            }

            case TIMERR_NOCANDO:
            {
                /* optional error information? */
                DWORD error = GetLastError();

                fwprintf(stderr, L"ERR => The ptc parameter is NULL, or the cbtc parameter is invalid.\n");
                
                if (sizeof(error) > sizeof(DWORD)) fwprintf(stderr, L"ERR => %s\n", returnMsgBuffer(error));

                return -1;
            }
        }
    }

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