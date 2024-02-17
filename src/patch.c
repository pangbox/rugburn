#include "patch.h"
#include "third_party/lend/ld32.h"

static PFNWRITEPROCESSMEMORYPROC pWriteProcessMemory = NULL;
static PFNFLUSHINSTRUCTIONCACHEPROC pFlushInstructionCache = NULL;
static PFNVIRTUALPROTECTPROC pVirtualProtect = NULL;
static PFNGETCURRENTPROCESSPROC pGetCurrentProcess = NULL;

VOID InitPatch() {
    pWriteProcessMemory = GetProc(hKernel32Module, "WriteProcessMemory");
    pFlushInstructionCache = GetProc(hKernel32Module, "FlushInstructionCache");
    pVirtualProtect = GetProc(hKernel32Module, "VirtualProtect");
    pGetCurrentProcess = GetProc(hKernel32Module, "GetCurrentProcess");
}

/**
 * Patch is a small routine for patching arbitrary memory.
 */
VOID Patch(LPVOID dst, LPVOID src, DWORD size) {
    DWORD OldProtection;
    pVirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &OldProtection);
    memcpy(dst, src, size);
    pVirtualProtect(dst, size, OldProtection, &OldProtection);
}

/**
 * Installs a hook that redirects a function to a different function with a
 * compatible signature. This will destroy the first 6 bytes of the function,
 * so you will need to build a trampoline before installing the hook if you
 * want to be able to call the original function again.
 */
VOID InstallHook(PVOID pfnProc, PVOID pfnTargetProc) {
    DWORD dwOldProtect;
    DWORD dwRelAddr;
    CHAR pbHook[6] = {0xE9, 0x00, 0x00, 0x00, 0x00, 0xC3};

    // Unprotect function.
    if (pVirtualProtect(pfnProc, 6, PAGE_EXECUTE_READWRITE, &dwOldProtect) == 0) {
        FatalError("Failed to install hook: VirtualProtect failed. (%08x)", LastErr());
    }

    // Create and install hook.
    dwRelAddr = ((DWORD)pfnTargetProc - (DWORD)pfnProc - 5);
    memcpy(&pbHook[1], &dwRelAddr, 4);
    if (!pWriteProcessMemory(pGetCurrentProcess(), pfnProc, pbHook, 6, NULL)) {
        FatalError("Failed to install hook: WriteProcessMemory failed. (%08x)", LastErr());
    }

    // Re-protect function.
    if (pVirtualProtect(pfnProc, 6, dwOldProtect, &dwOldProtect) == 0) {
        FatalError("Failed to install hook: VirtualProtect failed. (%08x)", LastErr());
    }

    // Flush the icache. Otherwise, it is theoretically possible that our
    // patch will not work consistently.
    if (pFlushInstructionCache(pGetCurrentProcess(), pfnProc, 6) == 0) {
        FatalError("Failed to install hook: FlushInstructionCache failed. (%08x)", LastErr());
    }
}

/**
 * Counts at least minBytes bytes worth of instructions, ending at an
 * instruction boundary.
 */
DWORD CountOpcodeBytes(FARPROC fn, DWORD minBytes) {
    PBYTE fndata = (PBYTE)fn;
    DWORD count = 0;

    while (count < minBytes) {
        count += length_disasm(&fndata[count]);
    }

    return count;
}

/**
 * Builds a trampoline to be able to call a function that has been hooked.
 * Conceptually, it takes prefixLen bytes (at least 6 bytes) from the original
 * function, then places them into a small block of code that jumps that many
 * bytes into the original function call. prefixLen needs to be longer than 6,
 * but must end on an instruction boundary. In addition, this approach is
 * naive and won't always work well, though for most functions the prefix will
 * be in the prologue and should be safe.
 */
PCHAR BuildTrampoline(DWORD fn, DWORD prefixLen) {
    DWORD trampolineLen;
    DWORD oldProtect;
    DWORD relAddr;
    PCHAR codeblock;

    // Allocate a block of memory. We need to fit the prefix + a jump.
    // Extra byte is for a return so that the instruction after the jump will
    // be valid. I'm not sure if this is strictly necessary.
    trampolineLen = prefixLen + 6;
    codeblock = AllocMem(trampolineLen);

    // Copy the prefix into our newly minted codeblock.
    memcpy(codeblock, (void *)fn, prefixLen);

    // Calculate the jump address. We want to jump into the function at the same
    // point we left off here, so we can just subtract the size of the jmp
    // instruction and otherwise it's a flat subtraction.
    relAddr = (DWORD)fn - (DWORD)codeblock - 5;

    // Create our jump instruction.
    codeblock[prefixLen] = 0xE9;
    memcpy(&codeblock[prefixLen + 1], &relAddr, 4);

    // ...and a return at the end, for good measure.
    codeblock[prefixLen + 5] = 0xC3;

    // Mark the codeblock as executable.
    pVirtualProtect(codeblock, trampolineLen, PAGE_EXECUTE_READWRITE, &oldProtect);

    return codeblock;
}

/**
 * Creates a trampoline then hooks the provided function to call the target
 * instead.
 */
PVOID HookFunc(PVOID pfnProc, PVOID pfnTargetProc) {
    DWORD prefixLen, trampoline;

    prefixLen = CountOpcodeBytes(pfnProc, 6);

    trampoline = (DWORD)BuildTrampoline((DWORD)pfnProc, prefixLen);

    InstallHook((PCHAR)pfnProc, pfnTargetProc);

    return (PVOID)trampoline;
}

/**
 * Hooks a procedure in a module.
 */
PVOID HookProc(HMODULE hModule, LPCSTR szName, PVOID pfnTargetProc) {
    PVOID pfnLibraryProc, pfnTrampolineProc;

    pfnLibraryProc = GetProc(hModule, szName);

    pfnTrampolineProc = HookFunc(pfnLibraryProc, pfnTargetProc);

    return pfnTrampolineProc;
}

/**
 * Creates a thunk that translates from MSVC thiscall to stdcall calling
 * convention. The returned function pointer can be used in MSVC ABI vtables.
 */
PVOID BuildThiscallToStdcallThunk(PVOID pfnProc) {
    DWORD thunkLen = 9;
    DWORD oldProtect;
    DWORD relAddr;
    PCHAR codeblock;

    // Calculate the jump address.
    relAddr = (DWORD)pfnProc - (DWORD)codeblock;

    // Allocate data for thunk.
    codeblock = AllocMem(thunkLen);

    // Create calling convention thunk.
    // We want to put the this pointer, from ecx, onto the stack at the
    // left-most argument. Since we were called via stdcall, return address is
    // the current value on the stack, followed by left-most argument. So, pop
    // the return address, push the this pointer, and then push the return
    // address back onto the stack.
    codeblock[0] = 0x58; // pop eax
    codeblock[1] = 0x51; // push ecx
    codeblock[2] = 0x50; // push eax
    codeblock[3] = 0xE9; // jmp
    memcpy(&codeblock[4], &relAddr, 4);

    // ...and a return at the end, for good measure.
    codeblock[8] = 0xC3;

    // Mark the codeblock as executable.
    pVirtualProtect(codeblock, thunkLen, PAGE_EXECUTE_READWRITE, &oldProtect);

    return codeblock;
}

/**
 * Creates a thunk that translates from stdcall to MSVC thiscall calling
 * convention. The returned function pointer can be called using stdcall.
 */
PVOID BuildStdcallToThiscallThunk(PVOID pfnProc) {
    DWORD thunkLen = 9;
    DWORD oldProtect;
    DWORD relAddr;
    PCHAR codeblock;

    // Calculate the jump address.
    relAddr = (DWORD)pfnProc - (DWORD)codeblock;

    // Allocate data for thunk.
    codeblock = AllocMem(thunkLen);

    // Create calling convention thunk.
    // We want to put the this pointer, from the stack at the left-most
    // argument, into ecx. Since we were called via thiscall, return address is
    // the current value on the stack, followed by the this pointer argument.
    // So, pop the return address, pop the this pointer to ecx, and then push
    // the return address back onto the stack.
    codeblock[0] = 0x58; // pop eax
    codeblock[1] = 0x59; // pop ecx
    codeblock[2] = 0x50; // push eax
    codeblock[3] = 0xE9; // jmp
    memcpy(&codeblock[4], &relAddr, 4);

    // ...and a return at the end, for good measure.
    codeblock[8] = 0xC3;

    // Mark the codeblock as executable.
    pVirtualProtect(codeblock, thunkLen, PAGE_EXECUTE_READWRITE, &oldProtect);

    return codeblock;
}
