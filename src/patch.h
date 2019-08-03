#ifndef PATCH_H
#define PATCH_H

#include "common.h"

VOID RemotePatch(HANDLE hProcess, DWORD dwAddr, PBYTE pbData, PBYTE pbBackup, DWORD cbData);
VOID InstallHook(PVOID pfnProc, PVOID pfnTargetProc);
DWORD CountOpcodeBytes(FARPROC fn, DWORD minBytes);
PCHAR BuildTrampoline(DWORD fn, DWORD prefixLen);
PVOID HookFunc(PVOID pfnProc, PVOID pvTargetProc);
PVOID HookProc(HMODULE hModule, LPCSTR szName, PVOID pfnTargetProc);

#endif
