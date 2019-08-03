#include "inject.h"
#include "../../patch.h"
#include "../../inject.h"
#include "../../ntdll.h"

HMODULE hKernel32Module = NULL;
PFNISWOW64PROCESSPROC pIsWow64Process = NULL;
PFNCREATEPROCESSAPROC pCreateProcessA = NULL;

/**
 * CreateProcessAHook creates a process and hooks it with our library. This is
 * mainly used to kill GameGuard.des, but we can use it for all kinds of
 * shenanigans.
 */
BOOL STDCALL CreateProcessAHook(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation) {
    BOOL result;
    BOOL injectAtEntry = FALSE;
    BOOL needResume = FALSE;
    BOOL weAreWow = FALSE, theyAreWow = FALSE;

    // Kills off a weird GameGuard call that makes the game more annoying in
    // Wine (because it crashes.) For some reason, GameGuard tries to run:
    //     cmd /c "dxdiag /whql:off /x ./gameguard/npsys.des"
    if (lpCommandLine && lpCommandLine[0] == 'c' && lpCommandLine[1] == 'm' && lpCommandLine[2] == 'd' && lpCommandLine[3] == ' ') {
        return FALSE;
    }

    Log("CreateProcessA(%s, %s, %08x, %08x, %d, %08x, %p, %s, %p, %p);\r\n", lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);

    // We may need to keep the process suspended if that is requested.
    needResume = (dwCreationFlags & CREATE_SUSPENDED) == 0;

    // We always need to create suspended.
    dwCreationFlags &= CREATE_SUSPENDED;

    if (needResume && LoadNTDLL()) {
        // Inject the DLL after the process hits its original entrypoint.
        // More reliable but more complicated and needs internal undocumented
        // APIs. We can't do this for suspended createprocess because it would
        // muck with the state of the main thread too much.

        injectAtEntry = TRUE;
    }

    // Run original CreateProcessA. If it fails, we can just return early.
    result = pCreateProcessA(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
    if (!result) {
        return result;
    }

    if (pIsWow64Process) {
        pIsWow64Process(GetCurrentProcess(), &weAreWow);
        pIsWow64Process(lpProcessInformation->hProcess, &theyAreWow);
    }

    // Be careful: We can't inject ourselves if our WoW status differs.
    // This stops Internet Explorer from being injected and crashing.
    // If you want to inject into a 64-bit process, you will need a 64-bit
    // DLL to inject.
    if (weAreWow == theyAreWow) {
        if (injectAtEntry) {
            // This is more complicated, but more reliable. If we don't want for
            // the entrypoint, the process space will be nearly entirely
            // uninitialized, meaning nothing is guaranteed to work. If we inject
            // in this state, it may not work (It doesn't work on Windows XP.)
            JumpToEntrypoint(lpProcessInformation->hProcess, lpProcessInformation->hThread);
        }

        InjectProcess(lpProcessInformation->hProcess, GetSelfPath());
    }

    if (needResume) {
        ResumeThread(lpProcessInformation->hThread);
    }

    return result;
}

VOID InitInjectHook() {
    hKernel32Module = LoadLib("kernel32");
    pIsWow64Process = GetProc(hKernel32Module, "IsWow64Process");
    pCreateProcessA = HookFunc(CreateProcessA, CreateProcessAHook);
}
