#include "ntdll.h"

HMODULE hNTDLLModule = NULL;

PFNNTQUERYINFORMATIONPROCESSPROC NtQueryInformationProcess = NULL;

BOOL LoadNTDLL() {
    if (hNTDLLModule != NULL) {
        return TRUE;
    }

    hNTDLLModule = LoadLib("ntdll");
    if (hNTDLLModule == NULL) {
        return FALSE;
    }

    NtQueryInformationProcess = GetProc(hNTDLLModule, "NtQueryInformationProcess");
    if (NtQueryInformationProcess == NULL) {
        goto errFree;
    }

    return TRUE;

errFree:
    FreeLibrary(hNTDLLModule);
    hNTDLLModule = NULL;
    return FALSE;
}
