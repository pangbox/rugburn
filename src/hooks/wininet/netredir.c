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

#include "netredir.h"
#include "../../config.h"
#include "../../patch.h"

HMODULE hWinINet = NULL;
PFNINTERNETOPENURLAPROC pInternetOpenUrlA = NULL;

/**
 * InternetOpenUrlAHook redirects HTTP calls to localhost.
 */
HINTERNET STDCALL InternetOpenUrlAHook(HINTERNET hInternet, LPCSTR lpszUrl, LPCSTR lpszHeaders,
                                       DWORD dwHeadersLength, DWORD dwFlags, DWORD_PTR dwContext) {
    LPCSTR newURL;
    HINTERNET hResult;

    newURL = RewriteURL(lpszUrl);
    if (newURL != NULL) {
        Log("InternetOpenUrlA(%s -> %s)\r\n", lpszUrl, newURL);
        hResult =
            pInternetOpenUrlA(hInternet, newURL, lpszHeaders, dwHeadersLength, dwFlags, dwContext);
        FreeMem((HLOCAL)newURL);
    } else {
        Log("InternetOpenUrlA(%s) // (no rewrite rules matched)\r\n", lpszUrl, newURL);
        hResult =
            pInternetOpenUrlA(hInternet, newURL, lpszHeaders, dwHeadersLength, dwFlags, dwContext);
    }
    return hResult;
}

VOID InitNetRedirHook() {
    hWinINet = LoadLib("wininet");
    pInternetOpenUrlA = HookProc(hWinINet, "InternetOpenUrlA", InternetOpenUrlAHook);
}
