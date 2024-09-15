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

#include "window.h"
#include "../../patch.h"

static HMODULE hUser32Module = NULL;
static PFNCREATEWINDOWEXAPROC pCreateWindowExA = NULL;

/**
 * CreateWindowExAHook is a convenience patch that prevents PangYa from
 * creating topmost windows.
 */
static HWND STDCALL CreateWindowExAHook(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName,
                                        DWORD dwStyle, int X, int Y, int nWidth, int nHeight,
                                        HWND hWndParent, HMENU hMenu, HINSTANCE hInstance,
                                        LPVOID lpParam) {
    dwExStyle &= (~WS_EX_TOPMOST);
    return pCreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight,
                            hWndParent, hMenu, hInstance, lpParam);
}

VOID InitWindowHook() {
    hUser32Module = LoadLib("user32");
    pCreateWindowExA = (PFNCREATEWINDOWEXAPROC)HookProc(hUser32Module, "CreateWindowExA",
                                                        (PVOID)CreateWindowExAHook);
}
