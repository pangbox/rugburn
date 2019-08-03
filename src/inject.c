/*
 * This file contains code used for injecting
 */

#include "inject.h"
#include "ntdll.h"
#include "patch.h"
#include <stddef.h>

const IMAGE_ORDINAL_FLAG32 = 0x80000000;

typedef struct _DLL_INJECT {
    PVOID ImageBase;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_BASE_RELOCATION BaseRelocation;
    PIMAGE_IMPORT_DESCRIPTOR ImportDirectory;
    PFNLOADLIBRARYAPROC LoadLibraryA;
    PFNGETPROCADDRESSPROC GetProcAddress;
} DLL_INJECT, *PDLL_INJECT;

/**
 * Routine loads a DLL manually. The general algorithm is based on a
 * Stack Overflow answer by user SSpoke:
 * https://stackoverflow.com/a/55779290
 */
DWORD WINAPI LoadDll(PVOID p) {
    PDLL_INJECT injectData;
    HMODULE hModule, hKernel32;
    DWORD dwBaseDelta;
    PIMAGE_BASE_RELOCATION pIBR;
    PIMAGE_IMPORT_DESCRIPTOR pIID;
    PIMAGE_IMPORT_BY_NAME pIBN;
    PFNDLLMAINPROC pfnEntryPoint;

    injectData = (PDLL_INJECT)p;
    pIBR = injectData->BaseRelocation;
    dwBaseDelta = (DWORD)((LPBYTE)injectData->ImageBase - injectData->NtHeaders->OptionalHeader.ImageBase);

    // Relocate the image
    while (pIBR->VirtualAddress) {
        if (pIBR->SizeOfBlock >= sizeof(IMAGE_BASE_RELOCATION)) {
            DWORD i = 0;
            DWORD dwCount = (pIBR->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
            PWORD pwList = (PWORD)(pIBR + 1);
            PDWORD pdwPtr;
 
            for (i = 0; i < dwCount; i++) {
                if (pwList[i]) {
                    pdwPtr = (PDWORD)((LPBYTE)injectData->ImageBase + (pIBR->VirtualAddress+(pwList[i] & 0xFFF)));
                    *pdwPtr += dwBaseDelta;
                }
            }
        }

        pIBR = (PIMAGE_BASE_RELOCATION)((LPBYTE)pIBR + pIBR->SizeOfBlock);
    }

    // Perform runtime linking.
    pIID = injectData->ImportDirectory;
    while (pIID->Characteristics) {
        PIMAGE_THUNK_DATA FirstThunk, OrigFirstThunk;
        DWORD dwFnPtrAddr;

        OrigFirstThunk = (PIMAGE_THUNK_DATA)((LPBYTE)injectData->ImageBase + pIID->OriginalFirstThunk);
        FirstThunk = (PIMAGE_THUNK_DATA)((LPBYTE)injectData->ImageBase + pIID->FirstThunk);

        hModule = injectData->LoadLibraryA((LPCSTR)injectData->ImageBase + pIID->Name);
        if (!hModule) {
            return FALSE;
        }

        while (OrigFirstThunk->u1.AddressOfData) {
            if (OrigFirstThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG32) {
                // Function should be imported using ordinals.
                dwFnPtrAddr = (DWORD)injectData->GetProcAddress(hModule, (LPCSTR)(OrigFirstThunk->u1.Ordinal & 0xFFFF));
                if (!dwFnPtrAddr) {
                    return FALSE;
                }
                FirstThunk->u1.Function = dwFnPtrAddr;
            } else {
                // Function should be imported using name.
                pIBN = (PIMAGE_IMPORT_BY_NAME)((LPBYTE)injectData->ImageBase + OrigFirstThunk->u1.AddressOfData);
                dwFnPtrAddr = (DWORD)injectData->GetProcAddress(hModule, (LPCSTR)pIBN->Name);
                if (!dwFnPtrAddr) {
                    return FALSE;
                }
                FirstThunk->u1.Function = dwFnPtrAddr;
            }
            OrigFirstThunk++;
            FirstThunk++;
        }
        pIID++;
    }

    if (injectData->NtHeaders->OptionalHeader.AddressOfEntryPoint) {
        pfnEntryPoint = (PFNDLLMAINPROC)((LPBYTE)injectData->ImageBase + injectData->NtHeaders->OptionalHeader.AddressOfEntryPoint);
        return pfnEntryPoint((HMODULE)injectData->ImageBase,DLL_PROCESS_ATTACH,NULL);
    }

    return TRUE;
}

DWORD WINAPI LoadDllEnd() {
    return 0;
}

/**
 * Routine injects the DLL loading routine and runs it in a remote process.
 * The general algorithm is based on a Stack Overflow answer by user SSpoke:
 * https://stackoverflow.com/a/55779290
 */
VOID InjectProcess(HANDLE hProcess, LPCSTR dllName) {
    PIMAGE_DOS_HEADER pIDH;
    PIMAGE_NT_HEADERS pINH;
    PIMAGE_SECTION_HEADER pISH;
    HANDLE hThread, hFile;
    PVOID pvBuffer, pvImage, pvMem;
    DWORD dwFileSize, dwExitCode, dwRead, i;
    DLL_INJECT injectData = { 0 };

    hFile = CreateFile(dllName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        FatalError("Failed to open the DLL (%08x)", GetLastError());
    }

    dwFileSize = GetFileSize(hFile, NULL);
    pvBuffer = VirtualAlloc(NULL,  dwFileSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (!pvBuffer) {
        FatalError("Failed to allocate memory for DLL data (%08x)", GetLastError());
    }

    // Read the DLL
    if (!ReadFile(hFile, pvBuffer, dwFileSize, &dwRead, NULL)) {
        FatalError("Failed to read the DLL (%08x)", GetLastError());
    }

    CloseHandle(hFile);

    pIDH = (PIMAGE_DOS_HEADER)pvBuffer;

    if (pIDH->e_magic != IMAGE_DOS_SIGNATURE) {
        FatalError("Error: Invalid executable image.");
    }

    pINH = (PIMAGE_NT_HEADERS)((LPBYTE)pvBuffer+pIDH->e_lfanew);
    if (pINH->Signature != IMAGE_NT_SIGNATURE) {
        FatalError("Error: Invalid PE header.");
    }
    if ((pINH->FileHeader.Characteristics & IMAGE_FILE_DLL) != IMAGE_FILE_DLL) {
        FatalError("Error: The image is not DLL.");
    }

    // Allocate memory for the DLL.
    pvImage = VirtualAllocEx(hProcess, NULL, pINH->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!pvImage) {
        FatalError("Error: Unable to allocate memory for the DLL (%d)", GetLastError());
    }
 
    // Copy the header to target process
    if (!WriteProcessMemory(hProcess, pvImage, pvBuffer, pINH->OptionalHeader.SizeOfHeaders, NULL)) {
        FatalError("Error: Unable to copy headers to target process (%d)", GetLastError());
    }
 
    pISH = (PIMAGE_SECTION_HEADER)(pINH + 1);
 
    // Copy the DLL to target process
    for (i = 0; i < pINH->FileHeader.NumberOfSections; i++) {
        WriteProcessMemory(hProcess, (PVOID)((LPBYTE)pvImage + pISH[i].VirtualAddress), (PVOID)((LPBYTE)pvBuffer + pISH[i].PointerToRawData), pISH[i].SizeOfRawData, NULL);
    }
 
    // Allocate memory for the loader code
    pvMem = VirtualAllocEx(hProcess, NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
 
    if (!pvMem) {
        FatalError("Error: Unable to allocate memory for the loader code (%d)", GetLastError());
    }

    injectData.ImageBase = pvImage;
    injectData.NtHeaders = (PIMAGE_NT_HEADERS)((LPBYTE)pvImage + pIDH->e_lfanew);
    injectData.BaseRelocation = (PIMAGE_BASE_RELOCATION)((LPBYTE)pvImage + pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
    injectData.ImportDirectory = (PIMAGE_IMPORT_DESCRIPTOR)((LPBYTE)pvImage + pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    injectData.LoadLibraryA = LoadLibraryA;
    injectData.GetProcAddress = GetProcAddress;
 
    // Write the injection data to the target process.
    WriteProcessMemory(hProcess, pvMem, &injectData, sizeof(DLL_INJECT), NULL);
    
    // Write the loading routine to the target process.
    WriteProcessMemory(hProcess, (PVOID)((PDLL_INJECT)pvMem + 1), LoadDll, (DWORD)LoadDllEnd - (DWORD)LoadDll, NULL);

    // Create a remote thread to execute the loader code.
    hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)((PDLL_INJECT)pvMem + 1), pvMem, 0, NULL);
    if (!hThread) {
        FatalError("Error: Unable to execute loader code (%d)", GetLastError());
    }

    WaitForSingleObject(hThread, INFINITE);
    GetExitCodeThread(hThread, &dwExitCode);
    if (!dwExitCode) {
        FatalError("Error: Loader returned error (%d)", GetLastError());
    }
 
    CloseHandle(hThread);
    VirtualFreeEx(hProcess,pvMem, 0, MEM_RELEASE);
    VirtualFree(pvBuffer, 0, MEM_RELEASE);
}

/**
 * GetPeb finds the PEB address of a remote process.
 */
PPEB GetPeb(HANDLE ProcessHandle) {
    PROCESS_BASIC_INFORMATION processInfo = {0};
    NTSTATUS status;
    PPEB pPeb;

    status = NtQueryInformationProcess(ProcessHandle, 0x0, &processInfo, sizeof(processInfo), NULL);
    if (!NT_SUCCESS(status)) {
        return NULL;
    }

    return processInfo.PebBaseAddress;
}

/**
 * FindEntrypoint finds the entrypoint of a remote process. This works even if
 * the process is newly created and most things are not initialized yet.
 */
DWORD FindEntrypoint(HANDLE hProcess) {
    const bPeHeaderAddrOff = offsetof(IMAGE_DOS_HEADER, e_lfanew);
    const bPeEntryPointOff = offsetof(IMAGE_NT_HEADERS, OptionalHeader.AddressOfEntryPoint);

    PPEB pPeb;
    PVOID pImage, pEntry;
    PIMAGE_NT_HEADERS pNtHeaders;
    LONG peHeaderAddr;
    SIZE_T NumberOfBytesRead;

    pPeb = GetPeb(hProcess);

    if (!ReadProcessMemory(hProcess, &pPeb->Reserved3[1], &pImage, sizeof(pImage), NULL)) {
        FatalError("ReadProcessMemory failed (%08x)", GetLastError());
    }

    if (!ReadProcessMemory(hProcess, (PCHAR)pImage + bPeHeaderAddrOff, &peHeaderAddr, sizeof(peHeaderAddr), NULL)) {
        FatalError("ReadProcessMemory failed (%08x)", GetLastError());
    }

    pNtHeaders = (PIMAGE_NT_HEADERS)((PCHAR)pImage + peHeaderAddr);
    if (!ReadProcessMemory(hProcess, (PCHAR)pNtHeaders + bPeEntryPointOff, &pEntry, sizeof(pEntry), NULL)) {
        FatalError("ReadProcessMemory failed (%08x)", GetLastError());
    }

    pEntry = (PVOID)((PCHAR)pImage + (SIZE_T)pEntry);

    return (DWORD)pEntry;
}

/**
 * JumpToEntrypoint attempts to resume a process until it hits the breakpoint,
 * then suspend it again. This ensures the process space is initialized enough
 * for us to inject code in older NT-based OSes (like Windows XP.)
 */
VOID JumpToEntrypoint(HANDLE hProcess, HANDLE hThread) {
    CONTEXT context = {0};
    BYTE pbInfLoop[2] = {0xEB, 0xFE};
    BYTE pbOldEntry[3] = {0x00, 0x00};
    DWORD bEntry, i;

    // Patch the entrypoint.
    bEntry = FindEntrypoint(hProcess);
    RemotePatch(hProcess, bEntry, pbInfLoop, pbOldEntry, 2);
    ResumeThread(hThread);

    // Poll until we see the process at the entrypoint.
    for (i = 0; i < 100 && context.Eip != bEntry; ++i) {
        Sleep(50);
        context.ContextFlags = CONTEXT_CONTROL;
        GetThreadContext(hThread, &context);
    }

    // Suspend thread and unpatch.
    SuspendThread(hThread);
    RemotePatch(hProcess, bEntry, pbOldEntry, NULL, 2);
}
