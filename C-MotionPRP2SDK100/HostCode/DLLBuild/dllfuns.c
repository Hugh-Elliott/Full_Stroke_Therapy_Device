#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <windows.h>

#include "C-Motion.h"
#include "PMDdiag.h"
#include "PMDRPtypes.h"
#include "PMDperiph.h"
#include "PMDdevice.h"

#include "dllfuns.h"

//#define PMD_DEBUG               /* Not for release. */


static FILE *fplog = 0;                /* This will be used for debugging DLL behavior */

void test_log(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    if (fplog) {
        vfprintf(fplog, fmt, ap);
        fflush(fplog);
    }
    va_end(ap);
}


/* These procedures should eventually be migrated into the main line. */

/* The VB code should check that the return value is non-null. */
PMD_API PMDDeviceHandle * PMD_CCONV PMDDeviceAlloc(void)
{
    void *ret = malloc(sizeof(PMDDeviceHandle));
    test_log("DeviceAlloc %#x\n", ret);
    return (PMDDeviceHandle *)ret;
}

PMD_API void PMD_CCONV PMDDeviceFree(PMDDeviceHandle *hDev)
{
    free(hDev);
}

/* The VB code should check that the return value is non-null. */
PMD_API PMDPeriphHandle * PMD_CCONV PMDPeriphAlloc(void)
{
    void *ret = malloc(sizeof(PMDPeriphHandle));
    test_log("PeriphAlloc %#x\n", ret);
    return (PMDPeriphHandle *)ret;
}

PMD_API void PMD_CCONV PMDPeriphFree(PMDPeriphHandle *hPeriph)
{
    free(hPeriph);
}

/* The VB code should check that the return value is non-null. */
PMD_API PMDAxisHandle * PMD_CCONV PMDAxisAlloc(void)
{
    void *ret = malloc(sizeof(PMDAxisHandle));
    test_log("AxisAlloc %#x\n", ret);
    return (PMDAxisHandle *)ret;
}

PMD_API void PMD_CCONV PMDAxisFree(PMDAxisHandle *hAxis)
{
    free(hAxis);
}

/* The VB code should check that the return value is non-null. */
PMD_API PMDMemoryHandle * PMD_CCONV PMDMemoryAlloc(void)
{
    return (PMDMemoryHandle *)malloc(sizeof(PMDMemoryHandle));
}

PMD_API void PMD_CCONV PMDMemoryFree(PMDMemoryHandle *hMem)
{
    free(hMem);
}

PMD_API BOOL PMD_CCONV DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
#ifdef PMD_DEBUG
        if (!fplog)
            fplog = fopen("dlltest.log", "w");
        test_log("Process Attached!\n");
#endif
        break;
    case DLL_THREAD_ATTACH:
#ifdef PMD_DEBUG
        if (!fplog)
            fplog = fopen("dlltest.log", "w");
        test_log("Thread Attached!\n");
#endif
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
#ifdef PMD_DEBUG
        if (fplog) {
            fclose(fplog);
            fplog = 0;
        }
#endif
        break;
    }
    return TRUE;
}
