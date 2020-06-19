#ifndef INJECTOR_H
#define INJECTOR_H

#include "src/common.h"

void InjectProcess(HANDLE hProcess, LPCSTR pszDllName);

void JumpToEntrypoint(HANDLE hProcess, HANDLE hThread);

#endif
