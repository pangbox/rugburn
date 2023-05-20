#include "window.h"
#include "../../patch.h"

static HMODULE hUser32Module = NULL;
static PFNCREATEWINDOWEXAPROC pCreateWindowExA = NULL;

/**
 * CreateWindowExAHook is a convenience patch that prevents PangYa from
 * creating topmost windows.
 */
HWND STDCALL CreateWindowExAHook(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
    dwExStyle &= (~WS_EX_TOPMOST);
    return pCreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

VOID InitWindowHook() {
    hUser32Module = LoadLib("user32");
    pCreateWindowExA = HookProc(hUser32Module, "CreateWindowExA", CreateWindowExAHook);
}
