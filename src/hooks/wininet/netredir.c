#include "netredir.h"
#include "../../patch.h"
#include "../../config.h"

HMODULE hWinINet = NULL;
PFNINTERNETOPENURLAPROC pInternetOpenUrlA = NULL;

/**
 * InternetOpenUrlAHook redirects HTTP calls to localhost.
 */
HINTERNET STDCALL InternetOpenUrlAHook(HINTERNET hInternet, LPCSTR lpszUrl, LPCSTR lpszHeaders, DWORD dwHeadersLength, DWORD dwFlags, DWORD_PTR dwContext) {
    LPCSTR newURL;
    HINTERNET hResult;

    newURL = RewriteURL(lpszUrl);
    if (newURL != NULL) {
        Log("InternetOpenUrlA(%s -> %s)\r\n", lpszUrl, newURL);
        hResult = pInternetOpenUrlA(hInternet, newURL, lpszHeaders, dwHeadersLength, dwFlags, dwContext);
        FreeMem((HLOCAL)newURL);
    } else {
        Log("InternetOpenUrlA(%s) // (no rewrite rules matched)\r\n", lpszUrl, newURL);
        hResult = pInternetOpenUrlA(hInternet, newURL, lpszHeaders, dwHeadersLength, dwFlags, dwContext);
    }
    return hResult;
}

VOID InitNetRedirHook() {
    hWinINet = LoadLib("wininet");
    pInternetOpenUrlA = HookProc(hWinINet, "InternetOpenUrlA", InternetOpenUrlAHook);
}
