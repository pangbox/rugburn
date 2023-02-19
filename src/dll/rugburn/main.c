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

#include "../../bootstrap.h"
#include "../../common.h"
#include "../../config.h"
#include "../../ijlfwd.h"
#include "../../patch.h"
#include "../../hooks/hooks.h"

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

    pSetEnvironmentVariableA = GetProc(hKernel32Module, "SetEnvironmentVariableA");
    if (pSetEnvironmentVariableA("PANGYA_ARG", szPangyaArg) == 0) {
        FatalError("Couldn't set PANGYA_ARG (%08x)", LastErr());
    }
    FreeMem(szPangyaArg);
}

/**
 * Implements the GameGuard patches for Pangya US 852.00.
 */
static VOID STDCALL PatchGG_US852(PVOID unused) {
    while(1) {
        // TODO(john): Remove hardcoded addresses.
        if (*(DWORD*)0x00A495E0 == 0x8F143D83) {
            Patch((LPVOID)0x00A495E0, "\xC3\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A49670, "\xC3\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A49690, "\xC3\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A496B0, "\x90\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A496E0, "\xC3", 1);
            Patch((LPVOID)0x00A49840, "\xC3", 1);
            Log("Patched GG check routines (US 852)\r\n");
            return;
        }
        if (*(DWORD*)0x00A49580 == 0x8F143D83) {
            Patch((LPVOID)0x00A49580, "\xC3\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A49670, "\xC3\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A49690, "\xC3\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A496B0, "\x90\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A496E0, "\xC3", 1);
            Patch((LPVOID)0x00A49840, "\xC3", 1);
            Log("Patched GG check routines (US 824)\r\n");
            return;
        }
		Delay(5);
    }
}

/**
 * Implements the GameGuard patches for Pangya JP 972.00.
 */
static VOID STDCALL PatchGG_JP972(PVOID unused) {
    while(1) {
        // TODO(john): Remove hardcoded addresses.
        if (*(DWORD*)0x00A5CD10 == 0x1BA43D83) {
            Patch((LPVOID)0x00A5CD10, "\xC3\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A5CDA0, "\xC3\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A5CDC0, "\xC3\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A5CDE0, "\x90\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A5CE10, "\xC3", 1);
            Patch((LPVOID)0x00A5CE40, "\xC3", 1);
            Log("Patched GG check routines (JP 972)\r\n");
            return;
        }
        if (*(DWORD*)0x00A5CF80 == 0x1BA43D83) {
            Patch((LPVOID)0x00A5CF80, "\xC3\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A5C010, "\xC3\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A5C030, "\xC3\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A5C050, "\x90\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A5CE80, "\xC3", 1);
            Patch((LPVOID)0x00A5CEB0, "\xC3", 1);
            Log("Patched GG check routines (JP 974)\r\n");
            return;
        }
        if (*(DWORD*)0x00A5CF80 == 0x1C143D83) {
            Patch((LPVOID)0x00A5CF80, "\xC3\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A5C010, "\xC3\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A5C030, "\xC3\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A5C050, "\x90\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A5CE80, "\xC3", 1);
            Patch((LPVOID)0x00A5CEB0, "\xC3", 1);
            Log("Patched GG check routines (JP 983)\r\n");
            return;
        }
		Delay(5);
    }
}

/**
 * Initializes the GameGuard patches based on game version.
 */
static VOID InitGGPatch() {
    PFNCREATETHREADPROC pCreateThread;

    PANGYAVER pangyaVersion;
    LPTHREAD_START_ROUTINE patchThread = NULL;

    // TODO(john): Should support more versions of PangYa.
    pangyaVersion = DetectPangyaVersion();
    switch (pangyaVersion) {
    case PANGYA_US:
        patchThread = (LPTHREAD_START_ROUTINE)PatchGG_US852;
        break;
    case PANGYA_JP:
        patchThread = (LPTHREAD_START_ROUTINE)PatchGG_JP972;
        break;
    case PANGYA_TH:
        Warning("No GameGuard patch is available for PangyaTH.");
        break;
    }

    if (!patchThread) {
        Warning("It looks like no patch exists for this version of PangYa™.\nThe game will likely exit a couple minutes after detecting GameGuard is not present.");
        return;
    }

    pCreateThread = GetProc(hKernel32Module, "CreateThread");
    pCreateThread(0, 0, patchThread, 0, 0, 0);
}

// Standalone forwarding DLL entrypoint
extern BOOL STDCALL DllMain(HANDLE hInstance, DWORD dwReason, LPVOID reserved) {
    if (dwReason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    BootstrapPEB();
    InitCommon();
    InitPatch();

    InitLog();
    InitGGPatch();
    InitIJL15();
    InitEnvironment();
    InitHooks();

    return TRUE;
}

// Entrypoint for Slipstrm
extern BOOL STDCALL SlipstrmDllMain(HANDLE hInstance, DWORD dwReason, LPVOID reserved) {
    PFNDLLMAINPROC pSlipstreamOep;
    BOOL bOepResult;

    pSlipstreamOep = (PFNDLLMAINPROC)((*(DWORD*)((DWORD)hInstance + 0x40))+(DWORD)hInstance);

    // Call OEP; return failure if it fails.
    bOepResult = pSlipstreamOep(hInstance, dwReason, reserved);
    if (!bOepResult) {
        return FALSE;
    }

    if (dwReason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    BootstrapSlipstream((DWORD)hInstance);
    InitCommon();
    InitPatch();

    InitLog();
    InitGGPatch();
    InitEnvironment();
    InitHooks();

    return TRUE;
}
