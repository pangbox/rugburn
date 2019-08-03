#include "netredir.h"
#include "../../patch.h"

HMODULE hWinINet = NULL;
PFNINTERNETOPENURLAPROC pInternetOpenUrlA = NULL;

VOID RewriteURLHost(LPCSTR url, LPSTR buffer, LPCSTR host) {
    // TODO(john): should implement bounds checking or dynamic buffer.

    // Copy scheme
    while (*url && *url != '/') {
        *buffer++ = *url++;
    }

    // Copy slashes
    if (!*url) return;
    *buffer++ = *url++;
    if (!*url) return;
    *buffer++ = *url++;

    // Insert new hostname
    while (*host) *buffer++ = *host++;

    // Skip hostname
    while (*url && *url != ':' && *url != '/') url++;

    // Override port 80 -> 8080 for convenience.
    if (*url != ':') {
        *buffer++ = ':';
        *buffer++ = '8';
        *buffer++ = '0';
        *buffer++ = '8';
        *buffer++ = '0';
    }

    // Copy remainder
    while (*url) *buffer++ = *url++;
}

/**
 * InternetOpenUrlAHook redirects HTTP calls to localhost.
 */
HINTERNET STDCALL InternetOpenUrlAHook(HINTERNET hInternet, LPCSTR lpszUrl, LPCSTR lpszHeaders, DWORD dwHeadersLength, DWORD dwFlags, DWORD_PTR dwContext) {
    CHAR newURL[256] = {0};
    RewriteURLHost(lpszUrl, newURL, "127.0.0.1");
    Log("InternetOpenUrlA(%s -> %s)\r\n", lpszUrl, newURL);
    return pInternetOpenUrlA(hInternet, newURL, lpszHeaders, dwHeadersLength, dwFlags, dwContext);
}

VOID InitNetRedirHook() {
    hWinINet = LoadLib("wininet");
    pInternetOpenUrlA = HookProc(hWinINet, "InternetOpenUrlA", InternetOpenUrlAHook);
}
