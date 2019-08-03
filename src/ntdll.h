#ifndef NTDLL_H
#define NTDLL_H

#include "common.h"

typedef struct _PEB
{
    BYTE Reserved1[2];
    BYTE BeingDebugged;
    BYTE Reserved2[1];
    PVOID Reserved3[2];
} PEB, *PPEB;

typedef struct _PROCESS_BASIC_INFORMATION {
    PVOID Reserved1;
    PPEB PebBaseAddress;
    PVOID Reserved2[2];
    ULONG_PTR UniqueProcessId;
    PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;

extern PFNNTQUERYINFORMATIONPROCESSPROC NtQueryInformationProcess;

BOOL LoadNTDLL();

#endif
