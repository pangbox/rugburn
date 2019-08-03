#ifndef INJECT_H
#define INJECT_H

#include "common.h"

VOID InjectProcess(HANDLE hProcess, LPCSTR pszDllName);

VOID JumpToEntrypoint(HANDLE hProcess, HANDLE hThread);

#endif
