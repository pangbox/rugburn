/**
 * This is a shim for ijl15.dll that hooks various things. Its objectives are:
 *
 *  - To fully disable GameGuard.
 *  - To make using unmodified PangYa with custom servers easier.
 *
 * As much as possible is accomplished by hooking Windows APIs, making many of
 * the patches portable across Pangya versions. The only bits that aren't
 * implemented this way are a few bits related to disabling GameGuard, due to
 * the fact that the process will attempt to kill itself if GameGuard is not
 * present.
 */

#include "common.h"
#include "ijlfwd.h"
#include "hooks/hooks.h"

/**
 * InitEnvironment configures the PANGYA_ARG environment to avoid needing to
 * run the updater first.
 */
VOID InitEnvironment() {
    PANGYAVER pangyaVersion;
    PSTR szPangyaArg;

    pangyaVersion = DetectPangyaVersion();
    szPangyaArg = GetPangyaArg(pangyaVersion);

    if (SetEnvironmentVariableA("PANGYA_ARG", szPangyaArg) == 0) {
        FatalError("Couldn't set PANGYA_ARG (%08x)", GetLastError());
    }

    LocalFree(szPangyaArg);
}

/**
 * Patch is a small routine for patching arbitrary memory.
 */
VOID Patch(LPVOID dst, LPVOID src, DWORD size) {
    DWORD OldProtection;
    VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &OldProtection);
    memcpy(dst, src, size);
    VirtualProtect(dst, size, OldProtection, &OldProtection);
}

/**
 * Implements the GameGuard patches for Pangya US 852.00.
 */
VOID PatchGG_US852(LPVOID param) {
    while(1) {
        // TODO(john): Remove hardcoded addresses.
        if (*(DWORD*)0x00A495E0 == 0x8F143D83) {
            Patch((LPVOID)0x00A495E0, "\xC3\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A49670, "\xC3\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A49690, "\xC3\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A496B0, "\x90\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A496E0, "\xC3", 1);
            Patch((LPVOID)0x00A49840, "\xC3", 1);
            Log("Patched GG check routines (Pangya US 852)\r\n");
            return;
        }
		Sleep(5);
    }
}

/**
 * Implements the GameGuard patches for Pangya JP 972.00.
 */
VOID PatchGG_JP972(LPVOID param) {
    while(1) {
        // TODO(john): Remove hardcoded addresses.
        if (*(DWORD*)0x00A5CD10 == 0x1BA43D83) {
            Patch((LPVOID)0x00A5CD10, "\xC3\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A5CDA0, "\xC3\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A5CDC0, "\xC3\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A5CDE0, "\x90\x90\x90\x90\x90\x90\x90", 7);
            Patch((LPVOID)0x00A5CE10, "\xC3", 1);
            Patch((LPVOID)0x00A5CE40, "\xC3", 1);
            Log("Patched GG check routines (Pangya JP 972)\r\n");
            return;
        }
		Sleep(5);
    }
}

/**
 * Initializes the GameGuard patches based on game version.
 */
VOID InitGGPatch() {
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
    }

    if (!patchThread) {
        MessageBoxA(NULL, "It looks like no patch exists for this version of PangYa.\nThe game will likely exit a couple minutes after detecting GameGuard is not present.", "rugburn", MB_OK);
        return;
    }

    CreateThread(0, 0, patchThread, 0, 0, 0);
}

extern BOOL STDCALL DllMain(HANDLE hInstance, DWORD dwReason, LPVOID reserved) {
    if (dwReason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    LogInit();

    if (StrCmpA(pszSelfName, "GameGuard.des") == 0) {
        ExitProcess(0x755);
    }

    // TODO(john): how do we reason about whether we're PangYa or not?
    // Just looking at whether the filename is ProjectG or not is not very
    // reliable, that would be annoying. For now we just treat everything
    // that isn't GameGuard as ProjectG...

    InitGGPatch();
    InitIJL15();
    InitEnvironment();
    InitHooks();

    return TRUE;
}
