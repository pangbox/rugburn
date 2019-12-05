#include "inject.h"
#include "../../patch.h"
#include "../../ntdll.h"

HANDLE hGameguardFakeHandle;
HANDLE hOtherFakeHandle;

HMODULE hKernel32Module = NULL;
PFNCREATEPROCESSAPROC pCreateProcessA = NULL;
PFNGETEXITCODEPROCESSPROC pGetExitCodeProcess = NULL;

BOOL StrContains(LPCSTR lpHaystack, LPCSTR lpNeedle) {
    LPCSTR lpCheckHaystack, lpCheckNeedle;

    while (*lpHaystack != 0) {
        lpCheckHaystack = lpHaystack;
        lpCheckNeedle = lpNeedle;

        while (*lpCheckNeedle != 0) {
            if (*lpCheckHaystack == 0) {
                return FALSE;
            }
            if (*lpCheckHaystack != *lpCheckNeedle) {
                goto mismatch;
            }

            ++lpCheckNeedle;
            ++lpCheckHaystack;
        }
        return TRUE;

    mismatch:
        ++lpHaystack;
    }

    return FALSE;
}

BOOL STDCALL CreateProcessAHook(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation) {
    Log("CreateProcessA(%s, %s, %08x, %08x, %d, %08x, %p, %s, %p, %p);\r\n", lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
    if (lpApplicationName != NULL && StrContains(lpApplicationName, "GameGuard.des")) {
        lpProcessInformation->hProcess = hGameguardFakeHandle;
    } else {
        lpProcessInformation->hProcess = hOtherFakeHandle;
    }
    return TRUE;
}

BOOL STDCALL GetExitCodeProcessHook(HANDLE hProcess, LPDWORD lpExitCode) {
    Log("GetExitCodeProcess(%08x, %08x);\r\n", hProcess, lpExitCode);
    if (hProcess == hGameguardFakeHandle) {
        *lpExitCode = 0x755;
    } else if (hProcess == hOtherFakeHandle) {
        *lpExitCode = 0x0;
    } else {
        return pGetExitCodeProcess(hProcess, lpExitCode);
    }
    return TRUE;
}

VOID InitInjectHook() {
    hGameguardFakeHandle = CreateMutexA(NULL, FALSE, NULL);
    hOtherFakeHandle = CreateMutexA(NULL, FALSE, NULL);

    hKernel32Module = LoadLib("kernel32");
    pCreateProcessA = HookFunc(CreateProcessA, CreateProcessAHook);
    pGetExitCodeProcess = HookFunc(GetExitCodeProcess, GetExitCodeProcessHook);
}
