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

/**
 * This is a replacement for ijl15.dll that hooks various things. Its objectives are:
 *
 *  - To fully disable GameGuard.
 *  - To make using unmodified PangYa™ with custom servers easier.
 *
 * As much as possible is accomplished by hooking Windows APIs, making many of
 * the patches portable across PangYa™ versions. The only bits that aren't
 * implemented this way are a few bits related to disabling GameGuard, due to
 * the fact that the process will attempt to kill itself if GameGuard is not
 * present.
 */

#include "../../common.h"
#include "../../config.h"
#include "../../hooks/hooks.h"
#include "../../patch.h"

/**
 * InitEnvironment configures the PANGYA_ARG environment to avoid needing to
 * run the updater first.
 */
static VOID InitEnvironment() {
    PANGYAVER pangyaVersion;
    PSTR szPangyaArg;

    LoadJsonRugburnConfig();
    pangyaVersion = DetectPangyaVersion();
    szPangyaArg = GetPangyaArg(pangyaVersion);

    if (SetEnvironmentVariableA("PANGYA_ARG", szPangyaArg) == 0) {
        FatalError("Couldn't set PANGYA_ARG (%08x)", LastErr());
    }
    FreeMem(szPangyaArg);
}

extern BOOL STDCALL DllMain(HANDLE hInstance, DWORD dwReason, LPVOID reserved) {
    if (dwReason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    InitLog();
    InitEnvironment();
    InitHooks();

    return TRUE;
}
