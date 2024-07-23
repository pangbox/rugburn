#include "common.h"

#define LOG_FILENAME "gglog.txt"

static PSTR pszSelfPath = NULL;
static PSTR pszSelfName = NULL;
static PSTR pszLogPrefix = NULL;

static HMODULE hUser32Module = NULL;
static PFNWVSPRINTFAPROC pwvsprintfA = NULL;
static PFNMESSAGEBOXAPROC pMessageBoxA = NULL;

HMODULE hKernel32Module = NULL;
static PFNLOCALALLOCPROC pLocalAlloc = NULL;
static PFNLOCALFREEPROC pLocalFree = NULL;
static PFNGETMODULEFILENAMEAPROC pGetModuleFileNameA = NULL;
static PFNGETFILEATTRIBUTESAPROC pGetFileAttributesA = NULL;
static PFNCREATEFILEAPROC pCreateFileA = NULL;
static PFNEXITPROCESSPROC pExitProcess = NULL;
static PFNGETFILESIZEPROC pGetFileSize = NULL;
static PFNREADFILEPROC pReadFile = NULL;
static PFNWRITEFILEPROC pWriteFile = NULL;
static PFNCLOSEHANDLEPROC pCloseHandle = NULL;
static PFNLSTRCPYAPROC plstrcpyA = NULL;
static PFNGETCURRENTTHREADIDPROC pGetCurrentThreadId = NULL;
static PFNGETCURRENTPROCESSIDPROC pGetCurrentProcessId = NULL;
static PFNFREELIBRARYPROC pFreeLibrary = NULL;
static PFNSLEEPPROC pSleep = NULL;
static PFNGETSTDHANDLEPROC pGetStdHandle = NULL;

HMODULE hWinsock = NULL;
PFNHTONSPROC pHtons = NULL;
PFNGETADDRINFO pGetAddrInfo = NULL;
PFNFREEADDRINFO pFreeAddrInfo = NULL;
PFNVIRTUALQUERYPROC pVirtualQuery = NULL;
PFNGETLASTERRORPROC pGetLastError = NULL;
PFNGETMODULEHANDLEAPROC pGetModuleHandleA = NULL;

VOID InitCommon() {
    hUser32Module = LoadLib("user32");
    pMessageBoxA = GetProc(hUser32Module, "MessageBoxA");
    pwvsprintfA = GetProc(hUser32Module, "wvsprintfA");

    hKernel32Module = LoadLib("kernel32");
    pLocalAlloc = GetProc(hKernel32Module, "LocalAlloc");
    pLocalFree = GetProc(hKernel32Module, "LocalFree");
    pGetModuleHandleA = GetProc(hKernel32Module, "GetModuleHandleA");
    pGetModuleFileNameA = GetProc(hKernel32Module, "GetModuleFileNameA");
    pGetFileAttributesA = GetProc(hKernel32Module, "GetFileAttributesA");
    pCreateFileA = GetProc(hKernel32Module, "CreateFileA");
    pExitProcess = GetProc(hKernel32Module, "ExitProcess");
    pGetFileSize = GetProc(hKernel32Module, "GetFileSize");
    pReadFile = GetProc(hKernel32Module, "ReadFile");
    pWriteFile = GetProc(hKernel32Module, "WriteFile");
    pCloseHandle = GetProc(hKernel32Module, "CloseHandle");
    plstrcpyA = GetProc(hKernel32Module, "lstrcpyA");
    pGetCurrentThreadId = GetProc(hKernel32Module, "GetCurrentThreadId");
    pGetCurrentProcessId = GetProc(hKernel32Module, "GetCurrentProcessId");
    pGetLastError = GetProc(hKernel32Module, "GetLastError");
    pFreeLibrary = GetProc(hKernel32Module, "FreeLibrary");
    pSleep = GetProc(hKernel32Module, "Sleep");
    pGetStdHandle = GetProc(hKernel32Module, "GetStdHandle");
    pVirtualQuery = GetProc(hKernel32Module, "VirtualQuery");

    hWinsock = LoadLib("ws2_32");
    pHtons = GetProc(hWinsock, "htons");
    pGetAddrInfo = GetProc(hWinsock, "getaddrinfo");
    pFreeAddrInfo = GetProc(hWinsock, "freeaddrinfo");
}

#ifdef _MSC_VER
#pragma function(strcmp)
#pragma function(memcmp)
#pragma function(memcpy)
#pragma function(memset)
#endif

// MSVCRT runtime functions
#ifdef _MSC_VER
void __declspec(naked) _aullshr() {
    __asm {
        cmp         cl,40h
        jae         RETZERO
        cmp         cl,20h
        jae         MORE32
        shrd        eax,edx,cl
        shr         edx,cl
        ret
MORE32:
        mov         eax,edx
        xor         edx,edx
        and         cl,1Fh
        shr         eax,cl
        ret
RETZERO:
        xor         eax,eax
        xor         edx,edx
        ret
    }
}
#else
unsigned long long __stdcall _aullshr(unsigned long long a, long b) { return a >> b; }
#endif

// C standard library functions
int strcmp(LPCSTR dest, LPCSTR src) {
    int cmp;
    do {
        cmp = *dest - *src;
        if (cmp > 0) {
            return 1;
        } else if (cmp < 0) {
            return -1;
        }
    } while (*dest++ && *src++);
    return 0;
}

int memcmp(LPCVOID dest, LPCVOID src, size_t size) {
    LPCSTR bdest = (LPCSTR)dest;
    LPCSTR bsrc = (LPCSTR)src;
    int cmp;
    while (size > 0) {
        cmp = *bdest++ - *bsrc++;
        if (cmp > 0) {
            return 1;
        } else if (cmp < 0) {
            return -1;
        }
        --size;
    }
    return 0;
}

PVOID memcpy(PVOID dest, VOID const *src, size_t size) {
    BYTE *bdest = (BYTE *)dest;
    BYTE const *bsrc = (BYTE const *)src;
    while (size > 0) {
        *bdest++ = *bsrc++;
        --size;
    }
    return dest;
}

PVOID memset(PVOID p, INT c, UINT size) {
    PCHAR data = (PCHAR)p;
    while (size > 0) {
        *data++ = c;
        --size;
    }
    return p;
}

// String formatting
int VSPrintfZ(LPSTR dest, LPCSTR fmt, va_list args) {
    if (!pwvsprintfA) {
        *(DWORD *)0 = 0;
    }
    return pwvsprintfA(dest, fmt, args);
}

int SPrintfZ(LPSTR dest, LPCSTR fmt, ...) {
    int result;

    va_list args;
    va_start(args, fmt);
    result = VSPrintfZ(dest, fmt, args);
    va_end(args);

    return result;
}

// Memory allocation
PVOID AllocMem(size_t size) {
    if (!pLocalAlloc) {
        return NULL;
    }
    return pLocalAlloc(0, size);
}

VOID FreeMem(PVOID mem) {
    if (!pLocalFree) {
        return;
    }
    pLocalFree(mem);
}

LPSTR DupStr(LPCSTR src) {
    LPSTR str;
    LPSTR p;
    int len = 0;

    while (src[len])
        len++;
    str = AllocMem(len + 1);
    p = str;
    while (*src)
        *p++ = *src++;
    *p = '\0';
    return str;
}

// Utility functions
PSTR GetSelfPath() {
    if (pszSelfPath != NULL) {
        return pszSelfPath;
    }

    pszSelfPath = AllocMem(4096);

    pGetModuleFileNameA(pGetModuleHandleA("ijl15"), pszSelfPath, 4096);

    return pszSelfPath;
}

BOOL FileExists(LPCTSTR szPath) { return pGetFileAttributesA(szPath) != INVALID_FILE_ATTRIBUTES; }

LPSTR ReadEntireFile(LPCSTR szPath) {
    HANDLE hFile = NULL;
    DWORD dwBytesRead = 0, dwBytesToRead = 0;
    BOOL bErrorFlag = FALSE;
    LPSTR buffer = NULL, data = NULL;

    hFile = pCreateFileA(szPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        FatalError("Error opening file %s.", szPath);
        pExitProcess(1);
        return NULL;
    }

    dwBytesToRead = pGetFileSize(hFile, NULL);
    buffer = AllocMem(dwBytesToRead + 1);
    memset(buffer, 0, dwBytesToRead + 1);

    data = buffer;
    while (dwBytesToRead > 0) {
        bErrorFlag = pReadFile(hFile, data, dwBytesToRead, &dwBytesRead, NULL);

        if (!bErrorFlag) {
            FatalError("Error loading file %s.", szPath);
            break;
        }

        data += dwBytesRead;
        dwBytesToRead -= dwBytesRead;
    }

    pCloseHandle(hFile);
    return buffer;
}

VOID WriteEntireFile(LPCSTR szPath, LPCSTR data, DWORD dwBytesToWrite) {
    HANDLE hFile = NULL;
    DWORD dwBytesWritten;
    BOOL bErrorFlag = FALSE;

    hFile =
        pCreateFileA(szPath, FILE_WRITE_DATA, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        FatalError("Error creating file %s.", szPath);
        return;
    }

    while (dwBytesToWrite > 0) {
        bErrorFlag = pWriteFile(hFile, data, dwBytesToWrite, &dwBytesWritten, NULL);

        if (!bErrorFlag) {
            FatalError("Error writing to file %s.", szPath);
            break;
        }

        data += dwBytesWritten;
        dwBytesToWrite -= dwBytesWritten;
    }

    pCloseHandle(hFile);
}

VOID FatalError(PCHAR fmt, ...) {
    va_list args;
    PCHAR buffer;

    buffer = AllocMem(4096);

    // Cause a pagefault if we're unable to do anything.
    if (!buffer || !pMessageBoxA || !pExitProcess) {
        *(DWORD *)0 = 0;
    }

    va_start(args, fmt);
    VSPrintfZ(buffer, fmt, args);
    va_end(args);

    pMessageBoxA(HWND_DESKTOP, buffer, "rugburn", MB_OK | MB_ICONERROR);
    FreeMem(buffer);
    pExitProcess(1);
}

VOID Warning(PCHAR fmt, ...) {
    va_list args;
    PCHAR buffer = AllocMem(4096);

    va_start(args, fmt);
    VSPrintfZ(buffer, fmt, args);
    va_end(args);

    pMessageBoxA(HWND_DESKTOP, buffer, "rugburn", MB_OK | MB_ICONWARNING);
    FreeMem(buffer);
}

VOID Log(PCHAR fmt, ...) {
    va_list args;
    HANDLE hAppend;
    PCHAR buffer = AllocMem(4096);
    PCHAR pfxbuffer = AllocMem(128);
    PCHAR logmsg = buffer;
    DWORD cb = 0;

    if (pszLogPrefix != NULL) {
        plstrcpyA(logmsg, pszLogPrefix);
        while (*logmsg != '\0')
            logmsg++;
    }

    SPrintfZ(pfxbuffer, "T:%d] ", pGetCurrentThreadId());
    plstrcpyA(logmsg, pfxbuffer);
    while (*logmsg != '\0')
        logmsg++;

    cb = logmsg - buffer;

    va_start(args, fmt);
    cb += VSPrintfZ(logmsg, fmt, args);
    va_end(args);

    hAppend = pCreateFileA(LOG_FILENAME, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                           OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    pWriteFile(hAppend, buffer, cb, &cb, NULL);
    pCloseHandle(hAppend);
    FreeMem(buffer);
    FreeMem(pfxbuffer);
}

VOID InitLog() {
    PCHAR szPfxBuf = AllocMem(4096);
    PCHAR szFileName = AllocMem(4096);
    PCHAR szBaseName;
    DWORD cbFileName;

    cbFileName = pGetModuleFileNameA(NULL, szFileName, 4096);
    szBaseName = szFileName + cbFileName;

    while (szBaseName > szFileName) {
        if (*szBaseName == '\\') {
            szBaseName++;
            break;
        }
        szBaseName--;
    }

    pszSelfName = szBaseName;
    SPrintfZ(szPfxBuf, "%s:%d] ", szBaseName, pGetCurrentProcessId());
    pszLogPrefix = szPfxBuf;
}

VOID ConsoleLog(PCHAR fmt, ...) {
    va_list args;
    HANDLE hConsole;
    PCHAR buffer = AllocMem(4096);
    DWORD cb = 0;
    va_start(args, fmt);
    cb += VSPrintfZ(buffer, fmt, args);
    va_end(args);
    hConsole = pGetStdHandle((DWORD)-11);
    pWriteFile(hConsole, buffer, cb, &cb, NULL);
    FreeMem(buffer);
}

HMODULE LoadLib(LPCSTR szName) {
    HMODULE hModule;

    hModule = LoadLibraryA(szName);
    if (hModule == NULL) {
        FatalError("Could not load module %s (%08x)", szName, LastErr());
    }

    return hModule;
}

VOID FreeLib(HMODULE hModule) { pFreeLibrary(hModule); }

PVOID GetProc(HMODULE hModule, LPCSTR szName) {
    PVOID pvProc;

    pvProc = (PVOID)GetProcAddress(hModule, szName);
    if (pvProc == NULL) {
        FatalError("Could not load proc %s (%08x)", szName, LastErr());
    }

    return pvProc;
}

DWORD LastErr() { return pGetLastError(); }

VOID Delay(DWORD dwMilliseconds) { pSleep(dwMilliseconds); }

VOID Exit(DWORD dwExitCode) { pExitProcess(dwExitCode); }

PANGYAVER DetectPangyaVersion() {
    if (FileExists("PangyaUS.ini")) {
        return PANGYA_US;
    } else if (FileExists("japan.dat")) {
        return PANGYA_JP;
    } else if (FileExists("thailand.dat")) {
        return PANGYA_TH;
    } else if (FileExists("Pangya.ini")) {
        return PANGYA_KR;
    } else if (FileExists("brasil.dat")) {
        return PANGYA_BR;
    } else if (FileExists("taiwan.dat")) {
        return PANGYA_TW;
    } else if (FileExists("indonesia.dat")) {
        return PANGYA_ID;
    } else if (FileExists("sin.dat")) {
        return PANGYA_SEA;
    } else if (FileExists("German.dat")) {
        return PANGYA_EU;
    }
    return PANGYA_US;
}

PSTR GetPangyaArg(PANGYAVER pangyaVersion) {
    switch (pangyaVersion) {
    case PANGYA_US:
    case PANGYA_TW:
    default:
        return DupStr("{2D0FA24B-336B-4e99-9D30-3116331EFDA0}");

    case PANGYA_JP:
    case PANGYA_TH:
        return DupStr("{E69B65A2-7A7E-4977-85E5-B19516D885CB}");

    case PANGYA_EU: {

        if (FileExists("ProjectG_300eu+.pak"))
            return DupStr("{98C07F18-BB68-467e-8C2C-29F63771460A}");
        else if (FileExists("ProjetcG_400eu+.pak"))
            return DupStr("{EE3C542D-525E-4711-BD3B-588BBAB17426}");
        else
            return DupStr("{E69B65A2-7A7E-4977-85E5-B19516D885CB}");
    }
    }
}
