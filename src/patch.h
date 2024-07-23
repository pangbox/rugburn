/**
 * Copyright 2018-2024 John Chadwick <john@jchw.io>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose
 * with or without fee is hereby granted, provided that the above copyright notice
 * and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */

#ifndef PATCH_H
#define PATCH_H

#include "common.h"

VOID Patch(LPVOID dst, LPVOID src, DWORD size);
VOID InstallHook(PVOID pfnProc, PVOID pfnTargetProc);
DWORD CountOpcodeBytes(FARPROC fn, DWORD minBytes);
PCHAR BuildTrampoline(DWORD fn, DWORD prefixLen);
PVOID HookFunc(PVOID pfnProc, PVOID pvTargetProc);
PVOID HookProc(HMODULE hModule, LPCSTR szName, PVOID pfnTargetProc);
PVOID BuildThiscallToStdcallThunk(PVOID pfnProc);
PVOID BuildStdcallToThiscallThunk(PVOID pfnProc);
PVOID BuildStdcallToVirtualThiscallThunk(DWORD dwVtblOffset);

#endif
