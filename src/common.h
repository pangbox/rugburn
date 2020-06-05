#ifndef COMMON_H
#define COMMON_H

#define STDCALL __stdcall
#define EXPORT __export

#include <winsock2.h>
#include <windows.h>
#include <windns.h>
#include <wininet.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <subauth.h>

#include "common-fnptr.h"

typedef enum _PANGYAVER {
    PANGYA_US,
    PANGYA_JP,
    PANGYA_TH,
} PANGYAVER;

extern PCHAR pszSelfName;

int strcmp(LPCSTR dest, LPCSTR src);
int memcmp(LPCVOID dest, LPCVOID src, size_t size);
PVOID memcpy(PVOID dest, LPCVOID src, size_t size);
PVOID memset(PVOID p, INT c, UINT size);
BOOL FileExists(LPCSTR szPath);
LPSTR ReadEntireFile(LPCSTR szPath);
VOID WriteEntireFile(LPCSTR szPath, LPCSTR data, DWORD dwBytesToWrite);
VOID FatalError(PCHAR fmt, ...);
VOID Warning(PCHAR fmt, ...);
VOID Log(PCHAR fmt, ...);
VOID LogInit();
HMODULE LoadLib(LPCSTR szName);
PVOID GetProc(HMODULE hModule, LPCSTR szName);
PANGYAVER DetectPangyaVersion();
PSTR GetPangyaArg(PANGYAVER pangyaVersion);
PSTR GetSelfPath();

#endif
