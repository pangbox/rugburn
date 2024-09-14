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
PFNINTERNETCONNECTAPROC pInternetConnectA = NULL;
PFNHTTPOPENREQUESTAPROC pHttpOpenRequestA = NULL;
PFNHTTPSENDREQUESTAPROC pHttpSendRequestA = NULL;
PFNHTTPSENDREQUESTEXAPROC pHttpSendRequestExA = NULL;
PFNHTTPENDREQUESTAPROC pHttpEndRequestA = NULL;
PFNHTTPADDREQUESTHEADERSAPROC pHttpAddRequestHeadersA = NULL;
PFNHTTPQUERYINFOAPROC pHttpQueryInfoA = NULL;
PFNINTERNETCLOSEHANDLEPROC pInternetCloseHandle = NULL;
PFNINTERNETQUERYDATAAVAILABLEPROC pInternetQueryDataAvailable = NULL;
PFNINTERNETWRITEFILEPROC pInternetWriteFile = NULL;
PFNINTERNETREADFILEPROC pInternetReadFile = NULL;
PFNINTERNETQUERYOPTIONAPROC pInternetQueryOptionA = NULL;
PFNINTERNETSETOPTIONAPROC pInternetSetOptionA = NULL;
PFNINTERNETCRACKURLAPROC pInternetCrackUrlA = NULL;

typedef struct _http_header {
    LPCSTR lpszHeaders;
    DWORD dwHeaderLength;
    DWORD dwModifiers;
} http_header;

#define MAX_HTTP_HEADER 30

typedef struct _internet_ctx {
    struct _internet_ctx *_next;
    struct _internet_ctx *_prev;
    HINTERNET hOpen;
    HINTERNET hConnect;
    HINTERNET hNewConnect;
    HINTERNET hRequest;
    HINTERNET hNewRequest;
    LPSTR lpszMethod;
    LPCSTR *rgpszAcceptTypes;
    LPSTR lpszServerName;
    INTERNET_PORT nServerPort;
    LPSTR lpszUserName;
    LPSTR lpszPassword;
    DWORD dwService;
    DWORD dwFlags;
    DWORD_PTR dwContext;
    http_header vHeaders[MAX_HTTP_HEADER];
    DWORD nHeaders;
    LPCVOID lpFileBuffer;
    DWORD dwNumberOfBytesToWrite;
    LPINTERNET_BUFFERSA lpBuffersIn;
} internet_ctx;

static internet_ctx g_inet_ctx;

void insertInetCtx(internet_ctx *_ctx) {

    internet_ctx *node = g_inet_ctx._next;

    if (node == NULL) {
        g_inet_ctx._next = _ctx;
        _ctx->_next = NULL;
        _ctx->_prev = &g_inet_ctx;
    } else {

        while (node != NULL) {
            if (node->_next == NULL)
                break;
            node = node->_next;
        }

        node->_next = _ctx;
        _ctx->_prev = node;
        _ctx->_next = NULL;
    }
}

void createInetCtx(HINTERNET hOpen, HINTERNET hConnect, LPCSTR lpszServerName,
                   INTERNET_PORT nServerPort, LPCSTR lpszUserName, LPCSTR lpszPassword,
                   DWORD dwService, DWORD dwFlags, DWORD_PTR dwContext, HINTERNET hRequest,
                   HINTERNET hNewConnect) {

    internet_ctx *ctx = AllocMem(sizeof(internet_ctx));

    memset(ctx, 0, sizeof(*ctx));

    ctx->_next = NULL;
    ctx->_prev = NULL;
    ctx->hOpen = hOpen;
    ctx->hConnect = hConnect;
    ctx->hNewConnect = hNewConnect;
    ctx->hRequest = hRequest;
    ctx->hNewRequest = NULL;
    ctx->lpszMethod = NULL;
    ctx->lpszServerName = NULL;
    ctx->nServerPort = nServerPort;
    ctx->lpszUserName = NULL;
    ctx->lpszPassword = NULL;
    ctx->dwService = dwService;
    ctx->dwFlags = dwFlags;
    ctx->dwContext = dwContext;

    if (lpszServerName != NULL) {
        int len = lstrlenA(lpszServerName);

        ctx->lpszServerName = AllocMem(len + 1);
        memcpy(ctx->lpszServerName, lpszServerName, len);
        ctx->lpszServerName[len] = '\0';
    }

    if (lpszUserName != NULL) {
        int len = lstrlenA(lpszUserName);

        ctx->lpszUserName = AllocMem(len + 1);
        memcpy(ctx->lpszUserName, lpszUserName, len);
        ctx->lpszUserName[len] = '\0';
    }

    if (lpszPassword != NULL) {
        int len = lstrlenA(lpszPassword);

        ctx->lpszPassword = AllocMem(len + 1);
        memcpy(ctx->lpszPassword, lpszPassword, len);
        ctx->lpszPassword[len] = '\0';
    }

    insertInetCtx(ctx);
}

void destructInetCtx(internet_ctx *_ctx) {

    if (_ctx == NULL)
        return;

    if (_ctx->lpszMethod != NULL)
        FreeMem((HLOCAL)_ctx->lpszMethod);
    if (_ctx->lpszServerName != NULL)
        FreeMem((HLOCAL)_ctx->lpszServerName);
    if (_ctx->lpszUserName != NULL)
        FreeMem((HLOCAL)_ctx->lpszUserName);
    if (_ctx->lpszPassword != NULL)
        FreeMem((HLOCAL)_ctx->lpszPassword);

    memset(_ctx, 0, sizeof(internet_ctx));
}

void removeInetCtx(internet_ctx *_ctx) {

    if (_ctx == NULL || _ctx == &g_inet_ctx)
        return;

    // Invalid Node
    if (_ctx->_next == NULL && _ctx->_prev == NULL) {
        destructInetCtx(_ctx);
        FreeMem((HLOCAL)_ctx);
        return;
    }

    if (_ctx->_next != NULL)
        _ctx->_next->_prev = _ctx->_prev;
    if (_ctx->_prev != NULL)
        _ctx->_prev->_next = _ctx->_next;

    destructInetCtx(_ctx);
    FreeMem((HLOCAL)_ctx);
}

internet_ctx *findInetCtxByConnect(HINTERNET hConnect) {

    if (g_inet_ctx._next == NULL)
        return NULL;

    internet_ctx *node = g_inet_ctx._next;

    do {

        if (node->hConnect == hConnect)
            return node;

    } while ((node = node->_next) != NULL);

    return NULL;
}

internet_ctx *findInetCtxByRequest(HINTERNET hRequest) {

    if (g_inet_ctx._next == NULL)
        return NULL;

    internet_ctx *node = g_inet_ctx._next;

    do {

        if (node->hRequest == hRequest)
            return node;

    } while ((node = node->_next) != NULL);

    return NULL;
}

int PortLength(INTERNET_PORT _port) {

    if (_port == 80 || _port == 443)
        return 0;

    int length = 1; // ':' + 1

    if (_port < 10)
        return length + 1;
    if (_port < 100)
        return length + 2;
    if (_port < 1000)
        return length + 3;
    if (_port < 10000)
        return length + 4;
    if (_port < 100000)
        return length + 5;
    if (_port < 1000000)
        return length + 6;

    return length + 7;
}

int UserNameAndPasswordLength(DWORD _dwUserNameLength, DWORD _dwPasswordLength) {

    if (_dwUserNameLength == 0u && _dwPasswordLength == 0u)
        return 0;

    int length = _dwPasswordLength + _dwPasswordLength;

    if (_dwUserNameLength > 0u && _dwPasswordLength > 0u)
        length += 2;
    else
        length += 1;

    return length;
}

HINTERNET STDCALL InternetOpenUrlABypasSelfSignedCertificate(HINTERNET hInternet, LPCSTR lpszUrl,
                                                             LPCSTR lpszHeaders,
                                                             DWORD dwHeadersLength, DWORD dwFlags,
                                                             DWORD_PTR dwContext) {

    URL_COMPONENTSA url_cpsa;
    memset(&url_cpsa, 0, sizeof(URL_COMPONENTSA));
    url_cpsa.dwStructSize = sizeof(URL_COMPONENTSA);
    url_cpsa.dwSchemeLength = 10;
    url_cpsa.dwHostNameLength = 512;
    url_cpsa.dwUserNameLength = 256;
    url_cpsa.dwPasswordLength = 256;

    if (pInternetCrackUrlA(lpszUrl, lstrlenA(lpszUrl), 0, &url_cpsa) == FALSE) {
        Log("InternetCrackUrlA failed. Error: %d\r\n", LastErr());
        return NULL;
    }

    LPSTR lpszHostName =
        (url_cpsa.dwHostNameLength == 0u ? NULL : AllocMem(url_cpsa.dwHostNameLength + 1));

    if (lpszHostName != NULL) {
        memcpy(lpszHostName, url_cpsa.lpszHostName, url_cpsa.dwHostNameLength);
        lpszHostName[url_cpsa.dwHostNameLength] = '\0';
    }

    HINTERNET hConnect = pInternetConnectA(hInternet, lpszHostName, url_cpsa.nPort, NULL, NULL,
                                           INTERNET_SERVICE_HTTP, dwFlags, dwContext);

    if (hConnect == NULL) {
        Log("InternetConnectA. failed Error: %d\r\n", LastErr());
        if (lpszHostName != NULL)
            FreeMem((HLOCAL)lpszHostName);
        return NULL;
    }

    if (url_cpsa.nScheme == INTERNET_SCHEME_HTTPS)
        dwFlags |= INTERNET_FLAG_SECURE;

    if (Config.bBypassSelfSignedCertificate == TRUE)
        dwFlags |= INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;

    LPCSTR lpszNewObjectName =
        (lpszUrl + url_cpsa.dwSchemeLength + 3 +
         UserNameAndPasswordLength(url_cpsa.dwUserNameLength, url_cpsa.dwPasswordLength) +
         url_cpsa.dwHostNameLength + PortLength(url_cpsa.nPort));

    LPCSTR rgpszAcceptTypes[] = {"*/*", NULL};

    HINTERNET hReq = pHttpOpenRequestA(hConnect, "GET", lpszNewObjectName, HTTP_VERSIONA, NULL,
                                       rgpszAcceptTypes, dwFlags, dwContext);

    if (hReq == NULL) {
        Log("InternetOpenUrlABypassSeflsignedCertificate->HttpOpenRequestA. failed Error: %d\r\n",
            LastErr());
        pInternetCloseHandle(hConnect);
        if (lpszHostName != NULL)
            FreeMem((HLOCAL)lpszHostName);
        return hReq;
    }

    BOOL ret2 = FALSE;

    if (Config.bBypassSelfSignedCertificate == TRUE) {
        DWORD dwFlags2;
        DWORD dwBuffLen = sizeof(dwFlags2);

        ret2 = pInternetQueryOptionA(hReq, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags2, &dwBuffLen);

        if (ret2 == FALSE) {
            Log("InternetQueryOptionA failed. Error: %d\r\n", LastErr());
            pInternetCloseHandle(hReq);
            pInternetCloseHandle(hConnect);
            if (lpszHostName != NULL)
                FreeMem((HLOCAL)lpszHostName);
            return NULL;
        }

        dwFlags2 |= SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_WEAK_SIGNATURE |
                    SECURITY_FLAG_IGNORE_WRONG_USAGE | SECURITY_FLAG_IGNORE_REVOCATION;

        ret2 =
            pInternetSetOptionA(hReq, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags2, sizeof(dwFlags2));

        if (ret2 == FALSE) {
            Log("InternetSetOptionA failed. Error: %d\r\n", LastErr());
            pInternetCloseHandle(hReq);
            pInternetCloseHandle(hConnect);
            if (lpszHostName != NULL)
                FreeMem((HLOCAL)lpszHostName);
            return NULL;
        }
    }

    ret2 = pHttpSendRequestA(hReq, lpszHeaders, dwHeadersLength, NULL, 0);

    if (ret2 == FALSE) {
        Log("HttpSendRequestA. failed Error: %d\r\n", LastErr());
        pInternetCloseHandle(hReq);
        pInternetCloseHandle(hConnect);
        if (lpszHostName != NULL)
            FreeMem((HLOCAL)lpszHostName);
        return NULL;
    }

    DWORD statusCode = 0;
    DWORD dwStatusCodeLength = sizeof(statusCode);
    ret2 = pHttpQueryInfoA(hReq, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode,
                           &dwStatusCodeLength, NULL);
    if (ret2 == FALSE) {
        Log("HttpQueryInfoA(1). failed Error: %d\r\n", LastErr());
        pInternetCloseHandle(hReq);
        pInternetCloseHandle(hConnect);
        if (lpszHostName != NULL)
            FreeMem((HLOCAL)lpszHostName);
        return NULL;
    }

    if (statusCode == HTTP_STATUS_MOVED || statusCode == HTTP_STATUS_REDIRECT) {

        if (lpszHostName != NULL)
            FreeMem((HLOCAL)lpszHostName);

        DWORD dwBuffLength = 0;
        ret2 = pHttpQueryInfoA(hReq, HTTP_QUERY_LOCATION, NULL, &dwBuffLength, NULL);
        if (ret2 == FALSE && (LastErr() != ERROR_INSUFFICIENT_BUFFER || dwBuffLength == 0)) {
            Log("HttpQueryInfoA(2). failed Error: %d\r\n", LastErr());
            pInternetCloseHandle(hReq);
            pInternetCloseHandle(hConnect);
            return NULL;
        }

        LPSTR buff = AllocMem(dwBuffLength + 1);

        ret2 = pHttpQueryInfoA(hReq, HTTP_QUERY_LOCATION, buff, &dwBuffLength, NULL);
        if (ret2 == FALSE) {
            Log("HttpQueryInfoA(3). failed Error: %d\r\n", LastErr());
            pInternetCloseHandle(hReq);
            pInternetCloseHandle(hConnect);
            if (buff != NULL)
                FreeMem((HLOCAL)buff);
            return NULL;
        }

        pInternetCloseHandle(hReq);
        pInternetCloseHandle(hConnect);
        hReq = NULL;

        hReq = InternetOpenUrlABypasSelfSignedCertificate(hInternet, buff, lpszHeaders,
                                                          dwHeadersLength, dwFlags, dwContext);

        if (buff != NULL)
            FreeMem((HLOCAL)buff);

        return hReq;
    }

    // creates context to close the connect handle
    createInetCtx(NULL, hInternet, lpszHostName, url_cpsa.nPort, NULL, NULL, INTERNET_SERVICE_HTTP,
                  dwFlags, dwContext, hReq, hConnect);

    if (lpszHostName != NULL)
        FreeMem((HLOCAL)lpszHostName);

    return hReq;
}

/**
 * InternetOpenUrlAHook redirects HTTP calls to localhost.
 */
HINTERNET STDCALL InternetOpenUrlAHook(HINTERNET hInternet, LPCSTR lpszUrl, LPCSTR lpszHeaders,
                                       DWORD dwHeadersLength, DWORD dwFlags, DWORD_PTR dwContext) {
    LPCSTR newURL;
    LPCSTR refURL;
    HINTERNET hResult;

    // Manual redirect
    dwFlags |= INTERNET_FLAG_NO_AUTO_REDIRECT;

    refURL = newURL = RewriteURL(lpszUrl);
    if (newURL != NULL) {
        Log("InternetOpenUrlA(%s -> %s)\r\n", lpszUrl, newURL);
        hResult =
            pInternetOpenUrlA(hInternet, newURL, lpszHeaders, dwHeadersLength, dwFlags, dwContext);
    } else {
        newURL = lpszUrl;
        Log("InternetOpenUrlA(%s) // (no rewrite rules matched)\r\n", newURL);
        hResult =
            pInternetOpenUrlA(hInternet, newURL, lpszHeaders, dwHeadersLength, dwFlags, dwContext);
    }

    if (hResult == NULL) {
        DWORD error = LastErr();

        if (Config.bBypassSelfSignedCertificate == TRUE &&
            (error == ERROR_INTERNET_SEC_CERT_DATE_INVALID ||
             error == ERROR_INTERNET_SEC_CERT_CN_INVALID || error == ERROR_INTERNET_INVALID_CA))
            hResult = InternetOpenUrlABypasSelfSignedCertificate(
                hInternet, newURL, lpszHeaders, dwHeadersLength, dwFlags, dwContext);

    } else {
        DWORD statusCode = 0;
        DWORD dwStatusCodeLength = sizeof(statusCode);
        BOOL ret2 = pHttpQueryInfoA(hResult, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                                    &statusCode, &dwStatusCodeLength, NULL);
        if (ret2 == FALSE) {
            Log("HttpQueryInfoA(1). failed Error: %d\r\n", LastErr());
            if (refURL != NULL)
                FreeMem((HLOCAL)refURL);
            return hResult;
        }

        if (statusCode == HTTP_STATUS_MOVED || statusCode == HTTP_STATUS_REDIRECT) {
            DWORD dwBuffLength = 0;
            ret2 = pHttpQueryInfoA(hResult, HTTP_QUERY_LOCATION, NULL, &dwBuffLength, NULL);
            if (ret2 == FALSE && (LastErr() != ERROR_INSUFFICIENT_BUFFER || dwBuffLength == 0)) {
                Log("HttpQueryInfoA(2). failed Error: %d\r\n", LastErr());
                if (refURL != NULL)
                    FreeMem((HLOCAL)refURL);
                return hResult;
            }

            LPSTR buff = AllocMem(dwBuffLength + 1);

            ret2 = pHttpQueryInfoA(hResult, HTTP_QUERY_LOCATION, buff, &dwBuffLength, NULL);
            if (ret2 == FALSE) {
                Log("HttpQueryInfoA(3). failed Error: %d\r\n", LastErr());
                if (buff != NULL)
                    FreeMem((HLOCAL)buff);
                if (refURL != NULL)
                    FreeMem((HLOCAL)refURL);
                return hResult;
            }

            pInternetCloseHandle(hResult);
            hResult = NULL;

            hResult = InternetOpenUrlABypasSelfSignedCertificate(
                hInternet, buff, lpszHeaders, dwHeadersLength, dwFlags, dwContext);

            if (buff != NULL)
                FreeMem((HLOCAL)buff);
        }
    }

    if (refURL != NULL)
        FreeMem((HLOCAL)refURL);

    return hResult;
}

HINTERNET STDCALL InternetConnectAHook(HINTERNET hInternet, LPCSTR lpszServerName,
                                       INTERNET_PORT nServerPort, LPCSTR lpszUserName,
                                       LPCSTR lpszPassword, DWORD dwService, DWORD dwFlags,
                                       DWORD_PTR dwContext) {

    HINTERNET hConnect = pInternetConnectA(hInternet, lpszServerName, nServerPort, lpszUserName,
                                           lpszPassword, dwService, dwFlags, dwContext);

    if (hConnect != NULL)
        createInetCtx(hInternet, hConnect, lpszServerName, nServerPort, lpszUserName, lpszPassword,
                      dwService, dwFlags, dwContext, NULL, NULL);

    return hConnect;
}

HINTERNET STDCALL HttpOpenRequestAHook(HINTERNET hConnect, LPCSTR lpszVerb, LPCSTR lpszObjectName,
                                       LPCSTR lpszVersion, LPCSTR lpszReferrer,
                                       LPCSTR *lplpszAcceptTypes, DWORD dwFlags,
                                       DWORD_PTR dwContext) {

    HINTERNET hReq = pHttpOpenRequestA(hConnect, lpszVerb, lpszObjectName, lpszVersion,
                                       lpszReferrer, lplpszAcceptTypes, dwFlags, dwContext);

    internet_ctx *inet_ctx = findInetCtxByConnect(hConnect);

    if (hReq != NULL && inet_ctx != NULL) {
        DWORD len = 0;
        BOOL ret2 = pInternetQueryOptionA(hReq, INTERNET_OPTION_URL, NULL, &len);
        if (ret2 == FALSE && LastErr() != ERROR_INSUFFICIENT_BUFFER) {
            Log("InternetQueryOptionA(1) failed. Error: %d\r\n", LastErr());
            return hReq;
        }
        if (len <= 0) {
            Log("InternetQueryOptionA(1) returned the url length with the value 0. Error: %d\r\n",
                LastErr());
            return hReq;
        }
        LPSTR url = AllocMem(len);
        ret2 = pInternetQueryOptionA(hReq, INTERNET_OPTION_URL, url, &len);
        if (ret2 == FALSE) {
            FreeMem((HLOCAL)url);
            Log("InternetQueryOptionA(2) failed. Error: %d\r\n", LastErr());
            return hReq;
        }

        LPCSTR newURL = RewriteURL(url);
        if (newURL == NULL) {
            Log("HttpOpenRequestA(%s) // (no rewrite rules matched)\r\n", url);
            FreeMem((HLOCAL)url);
            return hReq;
        }

        Log("HttpOpenRequestA(%s -> %s)\r\n", url, newURL);

        FreeMem((HLOCAL)url);

        URL_COMPONENTSA url_cpsa;
        memset(&url_cpsa, 0, sizeof(URL_COMPONENTSA));
        url_cpsa.dwStructSize = sizeof(URL_COMPONENTSA);
        url_cpsa.dwSchemeLength = 10;
        url_cpsa.dwHostNameLength = 512;
        url_cpsa.dwUserNameLength = 256;
        url_cpsa.dwPasswordLength = 256;
        url_cpsa.dwUrlPathLength = 1024;
        url_cpsa.dwExtraInfoLength = 2048;

        if (pInternetCrackUrlA(newURL, lstrlenA(newURL), 0, &url_cpsa) == FALSE) {
            FreeMem((HLOCAL)newURL);
            Log("InternetCrackUrlA failed. Error: %d\r\n", LastErr());
            return hReq;
        }

        LPSTR lpszHostName =
            (url_cpsa.dwHostNameLength == 0u ? NULL : AllocMem(url_cpsa.dwHostNameLength + 1));
        LPSTR lpszUserName =
            (url_cpsa.dwUserNameLength == 0u ? NULL : AllocMem(url_cpsa.dwUserNameLength + 1));
        LPSTR lpszPassword =
            (url_cpsa.dwPasswordLength == 0u ? NULL : AllocMem(url_cpsa.dwPasswordLength + 1));

        if (lpszHostName != NULL) {
            memcpy(lpszHostName, url_cpsa.lpszHostName, url_cpsa.dwHostNameLength);
            lpszHostName[url_cpsa.dwHostNameLength] = '\0';
        }
        if (lpszUserName != NULL) {
            memcpy(lpszUserName, url_cpsa.lpszUserName, url_cpsa.dwUserNameLength);
            lpszUserName[url_cpsa.dwUserNameLength] = '\0';
        }
        if (lpszPassword != NULL) {
            memcpy(lpszPassword, url_cpsa.lpszPassword, url_cpsa.dwPasswordLength);
            lpszPassword[url_cpsa.dwPasswordLength] = '\0';
        }

        pInternetCloseHandle(hReq);
        hReq = NULL;

        inet_ctx->hNewConnect = pInternetConnectA(inet_ctx->hOpen, lpszHostName, url_cpsa.nPort,
                                                  lpszUserName, lpszPassword, inet_ctx->dwService,
                                                  inet_ctx->dwFlags, inet_ctx->dwContext);

        if (lpszHostName != NULL)
            FreeMem((HLOCAL)lpszHostName);
        if (lpszUserName != NULL)
            FreeMem((HLOCAL)lpszUserName);
        if (lpszPassword != NULL)
            FreeMem((HLOCAL)lpszPassword);

        if (inet_ctx->hNewConnect == NULL) {
            FreeMem((HLOCAL)newURL);
            Log("InternetConnectA failed. Error: %d\r\n", LastErr());
            return NULL;
        }

        LPCSTR lpszNewObjectName =
            (newURL + url_cpsa.dwSchemeLength + 3 +
             UserNameAndPasswordLength(url_cpsa.dwUserNameLength, url_cpsa.dwPasswordLength) +
             url_cpsa.dwHostNameLength + PortLength(url_cpsa.nPort));

        if (url_cpsa.nScheme == INTERNET_SCHEME_HTTPS)
            dwFlags |= INTERNET_FLAG_SECURE;

        if (Config.bBypassSelfSignedCertificate == TRUE)
            dwFlags |=
                INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;

        hReq = pHttpOpenRequestA(inet_ctx->hNewConnect, lpszVerb, lpszNewObjectName, lpszVersion,
                                 lpszReferrer, lplpszAcceptTypes, dwFlags, dwContext);

        FreeMem((HLOCAL)newURL);

        if (hReq == NULL) {
            pInternetCloseHandle(inet_ctx->hNewConnect);
            inet_ctx->hNewConnect = NULL;
            Log("HttpOpenRequestA failed. Error: %d\r\n", LastErr());
            return NULL;
        }

        if (Config.bBypassSelfSignedCertificate == TRUE) {
            DWORD dwFlags2;
            DWORD dwBuffLen = sizeof(dwFlags2);

            ret2 =
                pInternetQueryOptionA(hReq, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags2, &dwBuffLen);

            if (ret2 == FALSE) {
                Log("InternetQueryOptionA(3) failed. Error: %d\r\n", LastErr());
                pInternetCloseHandle(hReq);
                pInternetCloseHandle(inet_ctx->hNewConnect);
                inet_ctx->hNewConnect = NULL;
                return NULL;
            }

            dwFlags2 |= SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_WEAK_SIGNATURE |
                        SECURITY_FLAG_IGNORE_WRONG_USAGE | SECURITY_FLAG_IGNORE_REVOCATION;

            ret2 = pInternetSetOptionA(hReq, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags2,
                                       sizeof(dwFlags2));

            if (ret2 == FALSE) {
                Log("InternetSetOptionA failed. Error: %d\r\n", LastErr());
                pInternetCloseHandle(hReq);
                pInternetCloseHandle(inet_ctx->hNewConnect);
                inet_ctx->hNewConnect = NULL;
                return NULL;
            }
        }

        inet_ctx->hRequest = hReq;
        inet_ctx->rgpszAcceptTypes = lplpszAcceptTypes;

        if (lpszVerb != NULL) {
            int len = lstrlenA(lpszVerb);

            inet_ctx->lpszMethod = AllocMem(len + 1);
            memcpy(inet_ctx->lpszMethod, lpszVerb, len);
            inet_ctx->lpszMethod[len] = '\0';
        }
    }

    return hReq;
}

BOOL STDCALL HttpSendRequestACertificateInvalidRedirect(HINTERNET hRequest, LPCSTR lpszHeaders,
                                                        DWORD dwHeadersLength, LPVOID lpOptional,
                                                        DWORD dwOptionalLength) {

	internet_ctx *inet_ctx = findInetCtxByRequest(hRequest);

    if (inet_ctx == NULL)
        return FALSE;

    LPSTR url = NULL;
    BOOL ret2 = FALSE;

    if (LastErr() == ERROR_HTTP_REDIRECT_NEEDS_CONFIRMATION) {
        DWORD statusCode = 0;
        DWORD dwStatusCodeLength = sizeof(statusCode);
        ret2 = pHttpQueryInfoA((inet_ctx->hNewRequest != NULL ? inet_ctx->hNewRequest : hRequest),
                               HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                               &statusCode, &dwStatusCodeLength, NULL);
        if (ret2 == FALSE) {
            Log("HttpQueryInfoA(1). failed Error: %d\r\n", LastErr());
            return FALSE;
        }

        if (statusCode != HTTP_STATUS_MOVED && statusCode != HTTP_STATUS_REDIRECT)
            return FALSE;

        DWORD dwBuffLength = 0;
        ret2 = pHttpQueryInfoA((inet_ctx->hNewRequest != NULL ? inet_ctx->hNewRequest : hRequest),
                               HTTP_QUERY_LOCATION, NULL, &dwBuffLength, NULL);
        if (ret2 == FALSE && (LastErr() != ERROR_INSUFFICIENT_BUFFER || dwBuffLength == 0)) {
            Log("HttpQueryInfoA(2). failed Error: %d\r\n", LastErr());
            return FALSE;
        }

        url = AllocMem(dwBuffLength + 1);

        ret2 = pHttpQueryInfoA((inet_ctx->hNewRequest != NULL ? inet_ctx->hNewRequest : hRequest),
                               HTTP_QUERY_LOCATION, url, &dwBuffLength, NULL);
        if (ret2 == FALSE) {
            Log("HttpQueryInfoA(3). failed Error: %d\r\n", LastErr());
            if (url != NULL)
                FreeMem((HLOCAL)url);
            return FALSE;
        }

    } else {
        DWORD len = 0;
        ret2 = pInternetQueryOptionA(
            (inet_ctx->hNewRequest != NULL ? inet_ctx->hNewRequest : hRequest), INTERNET_OPTION_URL,
            NULL, &len);
        if (ret2 == FALSE && LastErr() != ERROR_INSUFFICIENT_BUFFER) {
            Log("InternetQueryOptionA(1) failed. Error: %d\r\n", LastErr());
            return FALSE;
        }
        if (len <= 0) {
            Log("InternetQueryOptionA(1) returned the url length with the value 0. Error: %d\r\n",
                LastErr());
            return FALSE;
        }
        url = AllocMem(len);
        ret2 = pInternetQueryOptionA(
            (inet_ctx->hNewRequest != NULL ? inet_ctx->hNewRequest : hRequest), INTERNET_OPTION_URL,
            url, &len);
        if (ret2 == FALSE) {
            FreeMem((HLOCAL)url);
            Log("InternetQueryOptionA(2) failed. Error: %d\r\n", LastErr());
            return FALSE;
        }
    }

    URL_COMPONENTSA url_cpsa;
    memset(&url_cpsa, 0, sizeof(URL_COMPONENTSA));
    url_cpsa.dwStructSize = sizeof(URL_COMPONENTSA);
    url_cpsa.dwSchemeLength = 10;
    url_cpsa.dwHostNameLength = 512;
    url_cpsa.dwUserNameLength = 256;
    url_cpsa.dwPasswordLength = 256;

    if (pInternetCrackUrlA(url, lstrlenA(url), 0, &url_cpsa) == FALSE) {
        Log("InternetCrackUrlA failed. Error: %d\r\n", LastErr());
        if (url != NULL)
            FreeMem((HLOCAL)url);
        return FALSE;
    }

    LPSTR lpszHostName =
        (url_cpsa.dwHostNameLength == 0u ? NULL : AllocMem(url_cpsa.dwHostNameLength + 1));

    if (lpszHostName != NULL) {
        memcpy(lpszHostName, url_cpsa.lpszHostName, url_cpsa.dwHostNameLength);
        lpszHostName[url_cpsa.dwHostNameLength] = '\0';
    }

    if (inet_ctx->hNewRequest != NULL) {
        pInternetCloseHandle(inet_ctx->hNewRequest);
        inet_ctx->hNewRequest = NULL;
    }
    if (inet_ctx->hNewConnect != NULL) {
        pInternetCloseHandle(inet_ctx->hNewConnect);
        inet_ctx->hNewConnect = NULL;
    }

    inet_ctx->hNewConnect =
        pInternetConnectA(inet_ctx->hOpen, lpszHostName, url_cpsa.nPort, NULL, NULL,
                          INTERNET_SERVICE_HTTP, inet_ctx->dwFlags, inet_ctx->dwContext);

    if (lpszHostName != NULL)
        FreeMem((HLOCAL)lpszHostName);

    if (inet_ctx->hNewConnect == NULL) {
        Log("InternetConnectA. failed Error: %d\r\n", LastErr());
        if (url != NULL)
            FreeMem((HLOCAL)url);
        return FALSE;
    }

    DWORD newFlags = inet_ctx->dwFlags;

    if (url_cpsa.nScheme == INTERNET_SCHEME_HTTPS)
        newFlags |= INTERNET_FLAG_SECURE;

    if (Config.bBypassSelfSignedCertificate == TRUE)
        newFlags |= INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;

    LPCSTR lpszNewObjectName =
        (url + url_cpsa.dwSchemeLength + 3 +
         UserNameAndPasswordLength(url_cpsa.dwUserNameLength, url_cpsa.dwPasswordLength) +
         url_cpsa.dwHostNameLength + PortLength(url_cpsa.nPort));

    inet_ctx->hNewRequest = pHttpOpenRequestA(
        inet_ctx->hNewConnect, inet_ctx->lpszMethod,
                                       lpszNewObjectName, HTTP_VERSIONA, NULL,
                          inet_ctx->rgpszAcceptTypes, newFlags, inet_ctx->dwContext);

    if (url != NULL)
        FreeMem((HLOCAL)url);

    if (inet_ctx->hNewRequest == NULL) {
        Log("HttpEndRequestACertificateInvalidRedirect->HttpOpenRequestA. failed Error: %d\r\n",
            LastErr());
        pInternetCloseHandle(inet_ctx->hNewConnect);
        inet_ctx->hNewConnect = NULL;
        return FALSE;
    }

    if (Config.bBypassSelfSignedCertificate == TRUE) {
        DWORD dwFlags2;
        DWORD dwBuffLen = sizeof(dwFlags2);

        ret2 = pInternetQueryOptionA(inet_ctx->hNewRequest, INTERNET_OPTION_SECURITY_FLAGS,
                                     &dwFlags2, &dwBuffLen);

        if (ret2 == FALSE) {
            Log("InternetQueryOptionA failed. Error: %d\r\n", LastErr());
            pInternetCloseHandle(inet_ctx->hNewRequest);
            pInternetCloseHandle(inet_ctx->hNewConnect);
            inet_ctx->hNewRequest = NULL;
            inet_ctx->hNewConnect = NULL;
            return FALSE;
        }

        dwFlags2 |= SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_WEAK_SIGNATURE |
                    SECURITY_FLAG_IGNORE_WRONG_USAGE | SECURITY_FLAG_IGNORE_REVOCATION;

        ret2 = pInternetSetOptionA(inet_ctx->hNewRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags2,
                                   sizeof(dwFlags2));

        if (ret2 == FALSE) {
            Log("InternetSetOptionA failed. Error: %d\r\n", LastErr());
            pInternetCloseHandle(inet_ctx->hNewRequest);
            pInternetCloseHandle(inet_ctx->hNewConnect);
            inet_ctx->hNewRequest = NULL;
            inet_ctx->hNewConnect = NULL;
            return FALSE;
        }
    }

    for (DWORD i = 0u; i < inet_ctx->nHeaders; i++) {

        ret2 = pHttpAddRequestHeadersA(inet_ctx->hNewRequest, inet_ctx->vHeaders[i].lpszHeaders,
                                       inet_ctx->vHeaders[i].dwHeaderLength,
                                       inet_ctx->vHeaders[i].dwModifiers);

        if (ret2 == FALSE) {
            Log("HttpAddRequestHeadersA failed. Error: %d\r\n", LastErr());
            pInternetCloseHandle(inet_ctx->hNewRequest);
            pInternetCloseHandle(inet_ctx->hNewConnect);
            inet_ctx->hNewRequest = NULL;
            inet_ctx->hNewConnect = NULL;
            return FALSE;
        }
    }

    ret2 = pHttpSendRequestA(inet_ctx->hNewRequest, lpszHeaders, dwHeadersLength, lpOptional,
                             dwOptionalLength);

    if (ret2 == FALSE) {
        DWORD error = LastErr();

        if (error == ERROR_HTTP_REDIRECT_NEEDS_CONFIRMATION ||
            (Config.bBypassSelfSignedCertificate == TRUE &&
             (error == ERROR_INTERNET_SEC_CERT_DATE_INVALID ||
              error == ERROR_INTERNET_SEC_CERT_CN_INVALID || error == ERROR_INTERNET_INVALID_CA))) {
            ret2 = HttpSendRequestACertificateInvalidRedirect(hRequest, lpszHeaders, dwHeadersLength,
                                                              lpOptional, dwOptionalLength);
        } else
            Log("HttpSendRequestA(2). failed Error: %d\r\n", error);

        pInternetCloseHandle(inet_ctx->hNewRequest);
        pInternetCloseHandle(inet_ctx->hNewConnect);
        inet_ctx->hNewRequest = NULL;
        inet_ctx->hNewConnect = NULL;
        return ret2;
    }

    return TRUE;
}

BOOL STDCALL HttpSendRequestAHook(HINTERNET hRequest, LPCSTR lpszHeaders, DWORD dwHeadersLength,
                                  LPVOID lpOptional, DWORD dwOptionalLength) {

    BOOL bRet = FALSE;

	internet_ctx *inet_ctx = findInetCtxByRequest(hRequest);

	if (inet_ctx == NULL)
		bRet = pHttpSendRequestA(hRequest, lpszHeaders, dwHeadersLength, lpOptional, dwOptionalLength);
	else {

		bRet = pHttpSendRequestA(
                (inet_ctx->hNewRequest != NULL ? inet_ctx->hNewRequest : hRequest), lpszHeaders,
                dwHeadersLength, lpOptional, dwOptionalLength);

		if (bRet == TRUE)
			return bRet;

		DWORD error = LastErr();

		if (error == ERROR_HTTP_REDIRECT_NEEDS_CONFIRMATION ||
			(Config.bBypassSelfSignedCertificate == TRUE &&
				(error == ERROR_INTERNET_SEC_CERT_DATE_INVALID ||
				error == ERROR_INTERNET_SEC_CERT_CN_INVALID || error == ERROR_INTERNET_INVALID_CA))) {

			bRet = HttpSendRequestACertificateInvalidRedirect(
				hRequest, lpszHeaders, dwHeadersLength, lpOptional, dwOptionalLength);
		}
	}

    return bRet;
}

BOOL STDCALL HttpSendRequestExAHook(HINTERNET hRequest, LPINTERNET_BUFFERSA lpBuffersIn,
                                    LPINTERNET_BUFFERSA lpBuffersOut, DWORD dwFlags,
                                    DWORD_PTR dwContext) {

    BOOL bRet = FALSE;
    
	internet_ctx *inet_ctx = findInetCtxByRequest(hRequest);

	if (inet_ctx == NULL)
		bRet = pHttpSendRequestExA(hRequest, lpBuffersIn, lpBuffersOut, dwFlags, dwContext);
	else {

		bRet = pHttpSendRequestExA(
                (inet_ctx->hNewRequest != NULL ? inet_ctx->hNewRequest : hRequest), lpBuffersIn,
                lpBuffersOut, dwFlags, dwContext);

		if (bRet == FALSE)
			return bRet;

		inet_ctx->lpBuffersIn = lpBuffersIn;
	}

    return bRet;
}

BOOL STDCALL HttpEndRequestACertificateInvalidRedirect(HINTERNET hRequest,
                                                       LPINTERNET_BUFFERSA lpBuffersOut,
                                                       DWORD dwFlags, DWORD_PTR dwContext) {

	internet_ctx *inet_ctx = findInetCtxByRequest(hRequest);

    if (inet_ctx == NULL)
        return FALSE;

    LPSTR url = NULL;
    BOOL ret2 = FALSE;

    if (LastErr() == ERROR_HTTP_REDIRECT_NEEDS_CONFIRMATION) {
        DWORD statusCode = 0;
        DWORD dwStatusCodeLength = sizeof(statusCode);
        ret2 = pHttpQueryInfoA((inet_ctx->hNewRequest != NULL ? inet_ctx->hNewRequest : hRequest),
                               HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                               &statusCode, &dwStatusCodeLength, NULL);
        if (ret2 == FALSE) {
            Log("HttpQueryInfoA(1). failed Error: %d\r\n", LastErr());
            return FALSE;
        }

        if (statusCode != HTTP_STATUS_MOVED && statusCode != HTTP_STATUS_REDIRECT)
            return FALSE;

        DWORD dwBuffLength = 0;
        ret2 = pHttpQueryInfoA((inet_ctx->hNewRequest != NULL ? inet_ctx->hNewRequest : hRequest),
                               HTTP_QUERY_LOCATION, NULL, &dwBuffLength, NULL);
        if (ret2 == FALSE && (LastErr() != ERROR_INSUFFICIENT_BUFFER || dwBuffLength == 0)) {
            Log("HttpQueryInfoA(2). failed Error: %d\r\n", LastErr());
            return FALSE;
        }

        url = AllocMem(dwBuffLength + 1);

        ret2 = pHttpQueryInfoA((inet_ctx->hNewRequest != NULL ? inet_ctx->hNewRequest : hRequest),
                               HTTP_QUERY_LOCATION, url, &dwBuffLength, NULL);
        if (ret2 == FALSE) {
            Log("HttpQueryInfoA(3). failed Error: %d\r\n", LastErr());
            if (url != NULL)
                FreeMem((HLOCAL)url);
            return FALSE;
        }

    } else {
        DWORD len = 0;
        ret2 = pInternetQueryOptionA(
            (inet_ctx->hNewRequest != NULL ? inet_ctx->hNewRequest : hRequest), INTERNET_OPTION_URL,
            NULL, &len);
        if (ret2 == FALSE && LastErr() != ERROR_INSUFFICIENT_BUFFER) {
            Log("InternetQueryOptionA(1) failed. Error: %d\r\n", LastErr());
            return FALSE;
        }
        if (len <= 0) {
            Log("InternetQueryOptionA(1) returned the url length with the value 0. Error: %d\r\n",
                LastErr());
            return FALSE;
        }
        url = AllocMem(len);
        ret2 = pInternetQueryOptionA(
            (inet_ctx->hNewRequest != NULL ? inet_ctx->hNewRequest : hRequest), INTERNET_OPTION_URL,
            url, &len);
        if (ret2 == FALSE) {
            FreeMem((HLOCAL)url);
            Log("InternetQueryOptionA(2) failed. Error: %d\r\n", LastErr());
            return FALSE;
        }
    }

    URL_COMPONENTSA url_cpsa;
    memset(&url_cpsa, 0, sizeof(URL_COMPONENTSA));
    url_cpsa.dwStructSize = sizeof(URL_COMPONENTSA);
    url_cpsa.dwSchemeLength = 10;
    url_cpsa.dwHostNameLength = 512;
    url_cpsa.dwUserNameLength = 256;
    url_cpsa.dwPasswordLength = 256;

    if (pInternetCrackUrlA(url, lstrlenA(url), 0, &url_cpsa) == FALSE) {
        Log("InternetCrackUrlA failed. Error: %d\r\n", LastErr());
        if (url != NULL)
            FreeMem((HLOCAL)url);
        return FALSE;
    }

    LPSTR lpszHostName =
        (url_cpsa.dwHostNameLength == 0u ? NULL : AllocMem(url_cpsa.dwHostNameLength + 1));

    if (lpszHostName != NULL) {
        memcpy(lpszHostName, url_cpsa.lpszHostName, url_cpsa.dwHostNameLength);
        lpszHostName[url_cpsa.dwHostNameLength] = '\0';
    }

    if (inet_ctx->hNewRequest != NULL) {
        pInternetCloseHandle(inet_ctx->hNewRequest);
        inet_ctx->hNewRequest = NULL;
    }
    if (inet_ctx->hNewConnect != NULL) {
        pInternetCloseHandle(inet_ctx->hNewConnect);
        inet_ctx->hNewConnect = NULL;
    }

    inet_ctx->hNewConnect = pInternetConnectA(inet_ctx->hOpen, lpszHostName, url_cpsa.nPort, NULL,
                                           NULL, INTERNET_SERVICE_HTTP, dwFlags, dwContext);

    if (lpszHostName != NULL)
        FreeMem((HLOCAL)lpszHostName);

    if (inet_ctx->hNewConnect == NULL) {
        Log("InternetConnectA. failed Error: %d\r\n", LastErr());
        if (url != NULL)
            FreeMem((HLOCAL)url);
        return FALSE;
    }

    DWORD newFlags = dwFlags;

    if (url_cpsa.nScheme == INTERNET_SCHEME_HTTPS)
        newFlags |= INTERNET_FLAG_SECURE;

    if (Config.bBypassSelfSignedCertificate == TRUE)
        newFlags |= INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;

    LPCSTR lpszNewObjectName =
        (url + url_cpsa.dwSchemeLength + 3 +
         UserNameAndPasswordLength(url_cpsa.dwUserNameLength, url_cpsa.dwPasswordLength) +
         url_cpsa.dwHostNameLength + PortLength(url_cpsa.nPort));

    inet_ctx->hNewRequest =
        pHttpOpenRequestA(inet_ctx->hNewConnect, inet_ctx->lpszMethod, lpszNewObjectName,
                          HTTP_VERSIONA, NULL,
                          inet_ctx->rgpszAcceptTypes, newFlags, dwContext);

    if (url != NULL)
        FreeMem((HLOCAL)url);

    if (inet_ctx->hNewRequest == NULL) {
        Log("HttpEndRequestACertificateInvalidRedirect->HttpOpenRequestA. failed Error: %d\r\n",
            LastErr());
        pInternetCloseHandle(inet_ctx->hNewConnect);
        inet_ctx->hNewConnect = NULL;
        return FALSE;
    }

    if (Config.bBypassSelfSignedCertificate == TRUE) {
        DWORD dwFlags2;
        DWORD dwBuffLen = sizeof(dwFlags2);

        ret2 = pInternetQueryOptionA(inet_ctx->hNewRequest, INTERNET_OPTION_SECURITY_FLAGS,
                                     &dwFlags2, &dwBuffLen);

        if (ret2 == FALSE) {
            Log("InternetQueryOptionA failed. Error: %d\r\n", LastErr());
            pInternetCloseHandle(inet_ctx->hNewRequest);
            pInternetCloseHandle(inet_ctx->hNewConnect);
            inet_ctx->hNewRequest = NULL;
            inet_ctx->hNewConnect = NULL;
            return FALSE;
        }

        dwFlags2 |= SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_WEAK_SIGNATURE |
                    SECURITY_FLAG_IGNORE_WRONG_USAGE | SECURITY_FLAG_IGNORE_REVOCATION;

        ret2 = pInternetSetOptionA(inet_ctx->hNewRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags2,
                                   sizeof(dwFlags2));

        if (ret2 == FALSE) {
            Log("InternetSetOptionA failed. Error: %d\r\n", LastErr());
            pInternetCloseHandle(inet_ctx->hNewRequest);
            pInternetCloseHandle(inet_ctx->hNewConnect);
            inet_ctx->hNewRequest = NULL;
            inet_ctx->hNewConnect = NULL;
            return FALSE;
        }
    }

    for (DWORD i = 0u; i < inet_ctx->nHeaders; i++) {

        ret2 = pHttpAddRequestHeadersA(inet_ctx->hNewRequest, inet_ctx->vHeaders[i].lpszHeaders,
                                       inet_ctx->vHeaders[i].dwHeaderLength,
                                       inet_ctx->vHeaders[i].dwModifiers);

        if (ret2 == FALSE) {
            Log("HttpAddRequestHeadersA failed. Error: %d\r\n", LastErr());
            pInternetCloseHandle(inet_ctx->hNewRequest);
            pInternetCloseHandle(inet_ctx->hNewConnect);
            inet_ctx->hNewRequest = NULL;
            inet_ctx->hNewConnect = NULL;
            return FALSE;
        }
    }

    ret2 = pHttpSendRequestExA(inet_ctx->hNewRequest, inet_ctx->lpBuffersIn, NULL, dwFlags,
                               inet_ctx->dwContext);
    if (ret2 == FALSE) {
        Log("HttpSendRequestExA. failed Error: %d\r\n", LastErr());
        pInternetCloseHandle(inet_ctx->hNewRequest);
        pInternetCloseHandle(inet_ctx->hNewConnect);
        inet_ctx->hNewRequest = NULL;
        inet_ctx->hNewConnect = NULL;
        return FALSE;
    }

    if (inet_ctx->lpFileBuffer != NULL && inet_ctx->dwNumberOfBytesToWrite > 0) {
        DWORD dwNumbetOfBytesWritten = 0;

        ret2 = pInternetWriteFile(inet_ctx->hNewRequest, inet_ctx->lpFileBuffer,
                                  inet_ctx->dwNumberOfBytesToWrite,
                                  &dwNumbetOfBytesWritten);

        if (ret2 == FALSE && LastErr() != ERROR_INSUFFICIENT_BUFFER) {
            Log("InternetWriteFile. failed Error: %d\r\n", LastErr());
            pInternetCloseHandle(inet_ctx->hNewRequest);
            pInternetCloseHandle(inet_ctx->hNewConnect);
            inet_ctx->hNewRequest = NULL;
            inet_ctx->hNewConnect = NULL;
            return FALSE;
        }
    }

    ret2 = pHttpEndRequestA(inet_ctx->hNewRequest, lpBuffersOut, dwFlags, dwContext);

    if (ret2 == FALSE) {
        DWORD error = LastErr();

        if (error == ERROR_HTTP_REDIRECT_NEEDS_CONFIRMATION ||
            (Config.bBypassSelfSignedCertificate == TRUE &&
             (error == ERROR_INTERNET_SEC_CERT_DATE_INVALID ||
              error == ERROR_INTERNET_SEC_CERT_CN_INVALID || error == ERROR_INTERNET_INVALID_CA))) {
            ret2 =
                HttpEndRequestACertificateInvalidRedirect(hRequest, lpBuffersOut, dwFlags, dwContext);
        } else
            Log("HttpEndRequestA(2). failed Error: %d\r\n", error);

        pInternetCloseHandle(inet_ctx->hNewRequest);
        pInternetCloseHandle(inet_ctx->hNewConnect);
        inet_ctx->hNewRequest = NULL;
        inet_ctx->hNewConnect = NULL;
        return ret2;
    }

    return TRUE;
}

BOOL STDCALL HttpEndRequestAHook(HINTERNET hRequest, LPINTERNET_BUFFERSA lpBuffersOut,
                                 DWORD dwFlags, DWORD_PTR dwContext) {

    BOOL bRet = FALSE;
    
	internet_ctx *inet_ctx = findInetCtxByRequest(hRequest);

	if (inet_ctx == NULL)
		bRet = pHttpEndRequestA(hRequest, lpBuffersOut, dwFlags, dwContext);
	else {

		bRet =
                pHttpEndRequestA((inet_ctx->hNewRequest != NULL ? inet_ctx->hNewRequest : hRequest),
                                 lpBuffersOut, dwFlags, dwContext);

		if (bRet == TRUE)
			return bRet;

		DWORD error = LastErr();

		if (error == ERROR_HTTP_REDIRECT_NEEDS_CONFIRMATION ||
			(Config.bBypassSelfSignedCertificate == TRUE &&
				(error == ERROR_INTERNET_SEC_CERT_DATE_INVALID ||
				error == ERROR_INTERNET_SEC_CERT_CN_INVALID || error == ERROR_INTERNET_INVALID_CA))) {

			bRet = HttpEndRequestACertificateInvalidRedirect(hRequest, lpBuffersOut,
                                                                     dwFlags, dwContext);
		}
	}

    return bRet;
}

BOOL STDCALL HttpAddRequestHeadersAHook(HINTERNET hRequest, LPCSTR lpszHeaders,
                                        DWORD dwHeadersLength, DWORD dwModifiers) {

    BOOL bRet = FALSE;
    
	internet_ctx *inet_ctx = findInetCtxByRequest(hRequest);

	if (inet_ctx == NULL)
		bRet = pHttpAddRequestHeadersA(hRequest, lpszHeaders, dwHeadersLength, dwModifiers);
	else {

		bRet = pHttpAddRequestHeadersA(
                (inet_ctx->hNewRequest != NULL ? inet_ctx->hNewRequest : hRequest), lpszHeaders,
                dwHeadersLength, dwModifiers);

		if (bRet == FALSE || inet_ctx->nHeaders >= MAX_HTTP_HEADER)
			return bRet;

		http_header *pHeader = &inet_ctx->vHeaders[inet_ctx->nHeaders++];

		pHeader->lpszHeaders = lpszHeaders;
		pHeader->dwHeaderLength = dwHeadersLength;
		pHeader->dwModifiers = dwModifiers;
	}

    return bRet;
}

BOOL STDCALL HttpQueryInfoAHook(HINTERNET hRequest, DWORD dwInfoLevel, LPVOID lpBuffer,
                                LPDWORD lpdwBufferLength, LPDWORD lpdwIndex) {

    BOOL bRet = FALSE;

	internet_ctx *inet_ctx = findInetCtxByRequest(hRequest);

	if (inet_ctx == NULL)
        bRet = pHttpQueryInfoA(hRequest, dwInfoLevel, lpBuffer, lpdwBufferLength, lpdwIndex);
    else
        bRet =
            pHttpQueryInfoA((inet_ctx->hNewRequest != NULL ? inet_ctx->hNewRequest : hRequest),
                            dwInfoLevel, lpBuffer, lpdwBufferLength, lpdwIndex);

    return bRet;
}

BOOL STDCALL InternetQueryDataAvailableHook(HINTERNET hFile, LPDWORD lpdwNumberOfBytesAvailable,
                                            DWORD dwFlags, DWORD_PTR dwContext) {

    BOOL bRet = FALSE;

	internet_ctx *inet_ctx = findInetCtxByRequest(hFile);

	if (inet_ctx == NULL)
        bRet =
            pInternetQueryDataAvailable(hFile, lpdwNumberOfBytesAvailable, dwFlags, dwContext);
    else
        bRet = pInternetQueryDataAvailable(
            (inet_ctx->hNewRequest != NULL ? inet_ctx->hNewRequest : hFile),
            lpdwNumberOfBytesAvailable, dwFlags, dwContext);

    return bRet;
}

BOOL STDCALL InternetWriteFileHook(HINTERNET hFile, LPCVOID lpBuffer, DWORD dwNumberOfBytesToWrite,
                                   LPDWORD lpdwNumberOfBytesWritten) {

    BOOL bRet = FALSE;

	internet_ctx *inet_ctx = findInetCtxByRequest(hFile);

	if (inet_ctx == NULL)
		bRet = pInternetWriteFile(hFile, lpBuffer, dwNumberOfBytesToWrite, lpdwNumberOfBytesWritten);
	else {
        bRet =
            pInternetWriteFile((inet_ctx->hNewRequest != NULL ? inet_ctx->hNewRequest : hFile),
                                lpBuffer, dwNumberOfBytesToWrite, lpdwNumberOfBytesWritten);

		if (bRet == FALSE || (inet_ctx->lpFileBuffer != NULL && inet_ctx->dwNumberOfBytesToWrite > 0u))
			return bRet;

		inet_ctx->lpFileBuffer = lpBuffer;
		inet_ctx->dwNumberOfBytesToWrite = dwNumberOfBytesToWrite;
	}

    return bRet;
}

BOOL STDCALL InternetReadFileHook(HINTERNET hFile, LPVOID lpBuffer, DWORD dwNumberOfBytesToRead,
                                  LPDWORD lpdwNumberOfBytesRead) {

    BOOL bRet = FALSE;

    internet_ctx *inet_ctx = findInetCtxByRequest(hFile);

    if (inet_ctx == NULL)
        bRet = pInternetReadFile(hFile, lpBuffer, dwNumberOfBytesToRead, lpdwNumberOfBytesRead);
    else
        bRet = pInternetReadFile((inet_ctx->hNewRequest != NULL ? inet_ctx->hNewRequest : hFile),
                                 lpBuffer, dwNumberOfBytesToRead, lpdwNumberOfBytesRead);

    return bRet;
}

BOOL STDCALL InternetCloseHandleHook(HINTERNET hInternet) {

    internet_ctx *inet_ctx = findInetCtxByConnect(hInternet);

    if (inet_ctx != NULL) {

        if (inet_ctx->hNewRequest != NULL)
            pInternetCloseHandle(inet_ctx->hNewRequest);
        if (inet_ctx->hNewConnect != NULL)
            pInternetCloseHandle(inet_ctx->hNewConnect);

        removeInetCtx(inet_ctx);
    }

    return pInternetCloseHandle(hInternet);
}

VOID InitNetRedirHook() {
    memset(&g_inet_ctx, 0, sizeof(g_inet_ctx));

    hWinINet = LoadLib("wininet");
    pInternetQueryOptionA = GetProc(hWinINet, "InternetQueryOptionA");
    pInternetSetOptionA = GetProc(hWinINet, "InternetSetOptionA");
    pInternetCrackUrlA = GetProc(hWinINet, "InternetCrackUrlA");
    pInternetOpenUrlA = HookProc(hWinINet, "InternetOpenUrlA", InternetOpenUrlAHook);
    pInternetConnectA = HookProc(hWinINet, "InternetConnectA", InternetConnectAHook);
    pHttpOpenRequestA = HookProc(hWinINet, "HttpOpenRequestA", HttpOpenRequestAHook);
    pHttpSendRequestA = HookProc(hWinINet, "HttpSendRequestA", HttpSendRequestAHook);
    pHttpEndRequestA = HookProc(hWinINet, "HttpEndRequestA", HttpEndRequestAHook);
    pHttpAddRequestHeadersA =
        HookProc(hWinINet, "HttpAddRequestHeadersA", HttpAddRequestHeadersAHook);
	pHttpQueryInfoA = HookProc(hWinINet, "HttpQueryInfoA", HttpQueryInfoAHook);
    pHttpSendRequestExA = HookProc(hWinINet, "HttpSendRequestExA", HttpSendRequestExAHook);
    pInternetCloseHandle = HookProc(hWinINet, "InternetCloseHandle", InternetCloseHandleHook);
    pInternetQueryDataAvailable =
        HookProc(hWinINet, "InternetQueryDataAvailable", InternetQueryDataAvailableHook);
    pInternetWriteFile = HookProc(hWinINet, "InternetWriteFile", InternetWriteFileHook);
    pInternetReadFile = HookProc(hWinINet, "InternetReadFile", InternetReadFileHook);
}
