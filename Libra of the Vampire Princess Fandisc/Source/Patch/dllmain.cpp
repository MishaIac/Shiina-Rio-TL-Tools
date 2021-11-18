#include "util.h"
#include "util_detours.h"
#include <intrin.h>

auto pfnRioLoadFileEx = (LPVOID(*)(LPCSTR, LPDWORD))0x00405940;

LPVOID RioLoadFileEx(LPCSTR lpName, LPDWORD lpSize)
{
    auto name = ShiftJisToUcs2(lpName);
    LogWrite(L"Loaded file %s\r\n", (LPCWSTR)name);

    FILE* fp;
    fopen_s(&fp, lpName, "rb");
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        auto size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        if (size > 0)
        {
            PVOID ptr = GlobalAlloc(GPTR, size);
            if (ptr)
            {
                if (fread(ptr, size, 1, fp) == 1)
                {
                    LogWrite("%s replaced\r\n", lpName);
                    fclose(fp);
                    *lpSize = size;
                    return ptr;
                }
                GlobalFree(ptr);
            }
        }
        fclose(fp);
    }

    return pfnRioLoadFileEx(lpName, lpSize);
}

void InstallHooks()
{
    InlineHook(pfnRioLoadFileEx, RioLoadFileEx);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {

            DetourRestoreAfterWith();

            //LogInit(L"dev.log");

            InstallHooks();

            break;
        }
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

BOOL APIENTRY RioShiinaTL()
{
    return TRUE;
}
