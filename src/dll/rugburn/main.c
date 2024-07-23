/**
 * This is a shim for ijl15.dll that hooks various things. Its objectives are:
 *
 *  - To fully disable GameGuard.
 *  - To make using unmodified PangYa™ with custom servers easier.
 *
 * As much as possible is accomplished by hooking Windows APIs, making many of
 * the patches portable across PangYa™ versions. The only bits that aren't
 * implemented this way are a few bits related to disabling GameGuard, due to
 * the fact that the process will attempt to kill itself if GameGuard is not
 * present.
 */

#include "../../common.h"
#include "../../config.h"
#include "../../hooks/hooks.h"
#include "../../patch.h"

/**
 * InitEnvironment configures the PANGYA_ARG environment to avoid needing to
 * run the updater first.
 */
static VOID InitEnvironment() {
    PFNSETENVIRONMENTVARIABLEAPROC pSetEnvironmentVariableA;

    PANGYAVER pangyaVersion;
    PSTR szPangyaArg;

    LoadJsonRugburnConfig();
    pangyaVersion = DetectPangyaVersion();
    szPangyaArg = GetPangyaArg(pangyaVersion);

    if (SetEnvironmentVariableA("PANGYA_ARG", szPangyaArg) == 0) {
        FatalError("Couldn't set PANGYA_ARG (%08x)", LastErr());
    }
    FreeMem(szPangyaArg);
}

extern BOOL STDCALL DllMain(HANDLE hInstance, DWORD dwReason, LPVOID reserved) {
    if (dwReason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }


    InitLog();
    InitEnvironment();
    InitHooks();

    return TRUE;
}
