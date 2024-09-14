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

#include "msvcr100.h"
#include "../../patch.h"

typedef int(__cdecl *STRICMPFNPTR)(const char *s1, const char *s2);
typedef char *(__cdecl *SETLOCALEFNPTR)(int category, const char *locale);

static HMODULE hMsvcrModule = NULL;
static STRICMPFNPTR pStricmp = NULL;
static SETLOCALEFNPTR pSetLocale = NULL;

static int lower(int c) { return (c >= 'A' && c <= 'Z') ? c + 'a' - 'A' : c; }

static int __cdecl StricmpHook(const char *s1, const char *s2) {
    int result = 0;
    if (!s1 || !s2)
        return 0x7fffffff;
    while (!result) {
        result = lower(*s1) - lower(*s2);
        if (!*s1 || !*s2)
            break;
        s1++, s2++;
    }
    return result;
}

static VOID InitMsvcr100Hook() {
    const char *oldlocale;
    int result;

    hMsvcrModule = LoadLib("msvcr100");
    if (!hMsvcrModule) {
        Log("Not checking for Wine _stricmp bug: msvcr100 not found.\n");
        return;
    }

    pStricmp = GetProc(hMsvcrModule, "_stricmp");
    pSetLocale = GetProc(hMsvcrModule, "setlocale");
    oldlocale = pSetLocale(2, NULL);
    if (!pStricmp || !pSetLocale) {
        Log("Not checking for Wine _stricmp bug: msvcr100 functions not found.\n");
    }

    Log("Checking for Wine _stricmp bug in msvcr100; current locale: %s - Temporarily switching to "
        "Kor.\n",
        oldlocale);

    pSetLocale(2, "Kor");
    result = pStricmp("\xB3\xA1", "\xBC\xBC");
    pSetLocale(2, oldlocale);

    if (result == 0) {
        Log("Wine _stricmp bug detected; mitigating.\n");
        pStricmp = HookProc(hMsvcrModule, "_stricmp", StricmpHook);
    } else {
        Log("Wine _stricmp bug not detected.\n");
    }
}

static VOID InitMsvcr71Hook() {
    const char *oldlocale;
    int result;

    hMsvcrModule = LoadLib("msvcr71");
    if (!hMsvcrModule) {
        Log("Not checking for Wine _stricmp bug: msvcr71 not found.\n");
        return;
    }

    pStricmp = GetProc(hMsvcrModule, "_stricmp");
    pSetLocale = GetProc(hMsvcrModule, "setlocale");
    oldlocale = pSetLocale(2, NULL);
    if (!pStricmp || !pSetLocale) {
        Log("Not checking for Wine _stricmp bug: msvcr71 functions not found.\n");
    }

    Log("Checking for Wine _stricmp bug in msvcr71; current locale: %s - Temporarily switching to "
        "Kor.\n",
        oldlocale);

    pSetLocale(2, "Kor");
    result = pStricmp("\xB3\xA1", "\xBC\xBC");
    pSetLocale(2, oldlocale);

    if (result == 0) {
        Log("Wine _stricmp bug detected; mitigating.\n");
        pStricmp = HookProc(hMsvcrModule, "_stricmp", StricmpHook);
    } else {
        Log("Wine _stricmp bug not detected.\n");
    }
}

VOID InitMsvcrHook() {
    InitMsvcr71Hook();
    InitMsvcr100Hook();
}
