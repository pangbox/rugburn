#include "msvcr100.h"
#include "../../patch.h"

typedef int (__cdecl* STRICMPFNPTR)(const char *s1, const char *s2);
typedef char *(__cdecl* SETLOCALEFNPTR)(int category, const char *locale);

static HMODULE hMsvcr100Module = NULL;
static STRICMPFNPTR pStricmp = NULL;
static SETLOCALEFNPTR pSetLocale = NULL;

int lower(int c) {
    return (c >= 'A' && c <= 'Z') ? c + 'a' - 'A' : c;
}

int __cdecl StricmpHook(const char *s1, const char *s2) {
    int result = 0;
    if (!s1 || !s2) return 0x7fffffff;
    while (!result) {
        result = lower(*s1) - lower(*s2);
        if (!*s1 || !*s2) break;
        s1++, s2++;
    }
    return result;
}

VOID InitMsvcr100Hook() {
    const char *oldlocale;
    int result;

    hMsvcr100Module = LoadLib("msvcr100");
    if (!hMsvcr100Module) {
        Log("Not checking for Wine _stricmp bug: msvcr100 not found.\n");
        return;
    }

    pStricmp = GetProc(hMsvcr100Module, "_stricmp");
    pSetLocale = GetProc(hMsvcr100Module, "setlocale");
    oldlocale = pSetLocale(2, NULL);
    if (!pStricmp || !pSetLocale) {
        Log("Not checking for Wine _stricmp bug: msvcr100 functions not found.\n");
    }

    Log("Checking for Wine _stricmp bug; current locale: %s - Temporarily switching to Kor.\n", oldlocale);

    pSetLocale(2, "Kor");
    result = pStricmp("\xB3\xA1", "\xBC\xBC");
    pSetLocale(2, oldlocale);

    if (result == 0) {
        Log("Wine _stricmp bug detected; mitigating.\n");
        pStricmp = HookProc(hMsvcr100Module, "_stricmp", StricmpHook);
    } else {
        Log("Wine _stricmp bug not detected.\n");
    }
}

