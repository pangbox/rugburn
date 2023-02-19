#ifndef PATCH_H
#define PATCH_H

#include "common.h"

VOID InitPatch();
VOID Patch(LPVOID dst, LPVOID src, DWORD size);
VOID InstallHook(PVOID pfnProc, PVOID pfnTargetProc);
DWORD CountOpcodeBytes(FARPROC fn, DWORD minBytes);
PCHAR BuildTrampoline(DWORD fn, DWORD prefixLen);
PVOID HookFunc(PVOID pfnProc, PVOID pvTargetProc);
PVOID HookProc(HMODULE hModule, LPCSTR szName, PVOID pfnTargetProc);

#endif
