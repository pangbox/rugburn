/**
 * Copyright 2018-2024 John Chadwick <john@jchw.io>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose
 * with or without fee is hereby granted, provided that the above copyright notice
 * and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */

#include "inject.h"
#include "../../config.h"
#include "../../patch.h"

static HMODULE hKernel32Module;
static HANDLE hGameguardFakeHandle;

static PFNCREATEMUTEXAPROC pCreateMutexA;
static PFNCREATEPROCESSAPROC pCreateProcessA = NULL;
static PFNGETEXITCODEPROCESSPROC pGetExitCodeProcess = NULL;

static BOOL StrContains(LPCSTR lpHaystack, LPCSTR lpNeedle) {
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

static BOOL STDCALL CreateProcessAHook(LPCSTR lpApplicationName, LPSTR lpCommandLine,
                                       LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                       LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                       BOOL bInheritHandles, DWORD dwCreationFlags,
                                       LPVOID lpEnvironment, LPCSTR lpCurrentDirectory,
                                       LPSTARTUPINFOA lpStartupInfo,
                                       LPPROCESS_INFORMATION lpProcessInformation) {
    Log("CreateProcessA(%s, %s, %08x, %08x, %d, %08x, %p, %s, %p, %p);\r\n", lpApplicationName,
        lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags,
        lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
    if (lpApplicationName != NULL && StrContains(lpApplicationName, "GameGuard.des")) {
        lpProcessInformation->hProcess = hGameguardFakeHandle;
        return TRUE;
    }
    return pCreateProcessA(lpApplicationName, lpCommandLine, lpProcessAttributes,
                           lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment,
                           lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}

static BOOL STDCALL GetExitCodeProcessHook(HANDLE hProcess, LPDWORD lpExitCode) {
    Log("GetExitCodeProcess(%08x, %08x);\r\n", hProcess, lpExitCode);
    if (hProcess == hGameguardFakeHandle) {
        *lpExitCode = 0x755;
    } else {
        return pGetExitCodeProcess(hProcess, lpExitCode);
    }
    return TRUE;
}

VOID InitInjectHook() {
    hKernel32Module = LoadLib("kernel32");
    pCreateMutexA = (PFNCREATEMUTEXAPROC)GetProc(hKernel32Module, "CreateMutexA");
    hGameguardFakeHandle = pCreateMutexA(NULL, FALSE, NULL);
    pCreateProcessA = (PFNCREATEPROCESSAPROC)HookProc(hKernel32Module, "CreateProcessA",
                                                      (PVOID)CreateProcessAHook);
    pGetExitCodeProcess = (PFNGETEXITCODEPROCESSPROC)HookProc(hKernel32Module, "GetExitCodeProcess",
                                                              (PVOID)GetExitCodeProcessHook);
}
