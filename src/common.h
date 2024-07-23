#ifndef COMMON_H
#define COMMON_H

#define STDCALL __stdcall
#define EXPORT __export

// clang-format off
#include <winsock2.h>
#include <windows.h>
#include <windns.h>
#include <wininet.h>
// clang-format on
#include <shlwapi.h>
#include <strsafe.h>
#include <subauth.h>

#include "common-fnptr.h"

typedef enum _PANGYAVER {
    PANGYA_KR,
    PANGYA_JP,
    PANGYA_TH,
    PANGYA_ID,
    PANGYA_BR,
    PANGYA_TW,
    PANGYA_US,
    PANGYA_EU,
    PANGYA_SEA
} PANGYAVER;

// C standard library functions
int strcmp(LPCSTR dest, LPCSTR src);
int memcmp(LPCVOID dest, LPCVOID src, size_t size);
PVOID memcpy(PVOID dest, LPCVOID src, size_t size);
PVOID memset(PVOID p, INT c, UINT size);

// String formatting
int VSPrintfZ(LPSTR dest, LPCSTR fmt, va_list args);
int SPrintfZ(LPSTR dest, LPCSTR fmt, ...);

// Memory allocation
PVOID AllocMem(size_t size);
VOID FreeMem(PVOID mem);
LPSTR DupStr(LPCSTR src);

// Utility functions
BOOL FileExists(LPCSTR szPath);
LPSTR ReadEntireFile(LPCSTR szPath);
VOID WriteEntireFile(LPCSTR szPath, LPCSTR data, DWORD dwBytesToWrite);
VOID FatalError(PCHAR fmt, ...);
VOID Warning(PCHAR fmt, ...);
VOID Log(PCHAR fmt, ...);
VOID ConsoleLog(PCHAR fmt, ...);
VOID InitLog();
HMODULE LoadLib(LPCSTR szName);
PVOID GetProc(HMODULE hModule, LPCSTR szName);
DWORD LastErr();
VOID Delay(DWORD dwMilliseconds);
VOID Exit(DWORD dwExitCode);
PANGYAVER DetectPangyaVersion();
PSTR GetPangyaArg(PANGYAVER pangyaVersion);
PSTR GetSelfPath();

#endif
