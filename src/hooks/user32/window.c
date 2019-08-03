#include "window.h"
#include "../../patch.h"

PFNCREATEWINDOWEXAPROC pCreateWindowExA = NULL;

/**
 * CreateWindowExAHook is a convenience patch that prevents PangYa from
 * creating topmost windows.
 */
HWND STDCALL CreateWindowExAHook(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
    dwExStyle &= (~WS_EX_TOPMOST);
    return pCreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

VOID InitWindowHook() {
    pCreateWindowExA = HookFunc(CreateWindowExA, CreateWindowExAHook);
}
