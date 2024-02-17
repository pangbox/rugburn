#include "bootstrap.h"

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))

PFNLOADLIBRARYAPROC pLoadLibraryA = NULL;
PFNGETPROCADDRESSPROC pGetProcAddress = NULL;

#ifndef _MSC_VER
typedef struct _EXCEPTION_REGISTRATION_RECORD {
    struct _EXCEPTION_REGISTRATION_RECORD *Next;
    EXCEPTION_DISPOSITION *Handler;
} EXCEPTION_REGISTRATION_RECORD, *PEXCEPTION_REGISTRATION_RECORD;

typedef struct _NT_TIB {
    PEXCEPTION_REGISTRATION_RECORD ExceptionList;
    PVOID StackBase;
    PVOID StackLimit;
    PVOID SubSystemTib;
    union {
        PVOID FiberData;
        ULONG Version;
    };
    PVOID ArbitraryUserPointer;
    struct _NT_TIB *Self;
} NT_TIB, *PNT_TIB;
#endif

typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG Flags;
    USHORT LoadCount;
    USHORT TlsIndex;
    union {
        LIST_ENTRY HashLinks;
        struct {
            PVOID SectionPointer;
            ULONG CheckSum;
        };
    };
    union {
        ULONG TimeDateStamp;
        PVOID LoadedImports;
    };
    PVOID EntryPointActivationContext;
    PVOID PatchInformation;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

typedef struct _PEB_LDR_DATA {
    ULONG Length;
    UCHAR Initialized;
    PVOID SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
    PVOID EntryInProgress;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _PEB {
    BYTE Reserved1[2];
    BYTE BeingDebugged;
    BYTE Reserved2[1];
    PVOID Reserved3[2];
    PPEB_LDR_DATA Ldr;
} PEB, *PPEB;

typedef struct _TEB {
    PVOID Reserved1[12];
    PPEB ProcessEnvironmentBlock;
} TEB, *PTEB;

#ifdef __WATCOMC__
    extern __inline PTEB GetTEB(void);
    #pragma aux GetTEB = "mov eax, [fs:18h]" value [eax];
#else
    static PTEB GetTEB() {
        #if defined(_M_X64)
            PTEB tebPtr = (PTEB)__readgsqword(OFFSETOF(NT_TIB, Self));
        #else // x86
            PTEB tebPtr = (PTEB)__readfsdword(OFFSETOF(NT_TIB, Self));
        #endif
        return tebPtr;
    }
#endif

static PPEB GetPEB() {
    return GetTEB()->ProcessEnvironmentBlock;
}

static PVOID GetKernel32Base() {
    PLDR_DATA_TABLE_ENTRY entry;
    PLIST_ENTRY link;
    PPEB peb;

    peb = GetPEB();

    // (Only exists in WinNT.)
    link = peb->Ldr->InMemoryOrderModuleList.Flink;
    link = link->Flink; // Ntdll
    link = link->Flink; // Kernel32

    entry = CONTAINING_RECORD(link, LDR_DATA_TABLE_ENTRY, InMemoryOrderModuleList);
    return entry->DllBase;
}

VOID BootstrapPEB() {
    PCHAR pImageBase;
    PIMAGE_DOS_HEADER pDosHeader;
    PIMAGE_NT_HEADERS pNtHeader;
    PIMAGE_DATA_DIRECTORY pExportDirEntry;
    PIMAGE_EXPORT_DIRECTORY pExportDir;
    DWORD iNumberOfFunctions;
    DWORD iNumberOfNames;
    LPDWORD pAddressOfFunctions;
    LPWORD pAddressOfOrdinals;
    LPDWORD pAddressOfNames;
    int i;
    int iLoadLibraryAOrdinal = -1;
    int iGetProcAddressOrdinal = -1;
    
    pImageBase = (PCHAR)GetKernel32Base();
    pDosHeader = (PIMAGE_DOS_HEADER)pImageBase;

    pNtHeader = (PIMAGE_NT_HEADERS)(pImageBase + pDosHeader->e_lfanew);
    pExportDirEntry = &pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    pExportDir = (PIMAGE_EXPORT_DIRECTORY)(pImageBase + pExportDirEntry->VirtualAddress);

    iNumberOfFunctions = pExportDir->NumberOfFunctions;
    iNumberOfNames = pExportDir->NumberOfNames;
    pAddressOfFunctions = (LPDWORD)(pExportDir->AddressOfFunctions + pImageBase);
    pAddressOfOrdinals = (LPWORD)(pExportDir->AddressOfNameOrdinals + pImageBase);
    pAddressOfNames  = (LPDWORD)(pExportDir->AddressOfNames + pImageBase);

    for (i = 0; i < (int)iNumberOfNames; i++) {
        PCHAR pName = (PCHAR)(pAddressOfNames[i] + pImageBase);

        // Exit early iff we have the ordinals we're looking for.
        if (iLoadLibraryAOrdinal != -1 && iGetProcAddressOrdinal != -1) {
            break;
        }

        if (strcmp(pName, "LoadLibraryA") == 0) {
            iLoadLibraryAOrdinal = pAddressOfOrdinals[i];
        }

        if (strcmp(pName, "GetProcAddress") == 0) {
            iGetProcAddressOrdinal = pAddressOfOrdinals[i];
        }
    }

    pLoadLibraryA = (PFNLOADLIBRARYAPROC)(pImageBase + pAddressOfFunctions[iLoadLibraryAOrdinal]);
    pGetProcAddress = (PFNGETPROCADDRESSPROC)(pImageBase + pAddressOfFunctions[iGetProcAddressOrdinal]);
}

VOID BootstrapSlipstream(DWORD dwModuleAddress) {
    pLoadLibraryA = *(PFNLOADLIBRARYAPROC*)(dwModuleAddress + 0x4f000 + 5 * 4);
    pGetProcAddress = *(PFNGETPROCADDRESSPROC*)(dwModuleAddress + 0x4f000 + 6 * 4);
}
