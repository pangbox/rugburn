/**
 * Copyright 2018-2024 John Chadwick <john@jchw.io>
 * Copyright 2024 Acrisio Filho
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

#include "custommemory.h"

BOOL InitCustomMemory(LPCUSTOMMEMORY pCustomMemory) {
    if (pCustomMemory == NULL)
        return FALSE;

    if (pCustomMemory->name == NULL) {
        Warning("CustomMemory name is empty.");
        return FALSE;
    }
    if (pCustomMemory->valueLen == 0 && pCustomMemory->newvalueLen == 0) {
        Warning("CustomMemory length is empty.");
        return FALSE;
    }
    if (pCustomMemory->newvalueLen < pCustomMemory->valueLen)
        pCustomMemory->newvalueLen = pCustomMemory->valueLen;
    pCustomMemory->newvalue = (LPSTR)AllocMem(pCustomMemory->newvalueLen);
    if (pCustomMemory->newvalue == NULL) {
        Warning("CustomMemory failed to allocate memory size: %d.", pCustomMemory->newvalueLen);
        return FALSE;
    }
    memset(pCustomMemory->newvalue, 0, pCustomMemory->newvalueLen);
    memcpy(pCustomMemory->newvalue, pCustomMemory->value, pCustomMemory->valueLen);

    if (pCustomMemory->bExecuteMemory) {
        DWORD oldProtect;

        // Mark the newvalue as executable.
        if (VirtualProtect(pCustomMemory->newvalue, pCustomMemory->newvalueLen,
                           PAGE_EXECUTE_READWRITE, &oldProtect) == FALSE) {
            Warning("CustomMemory failed to mark memory as executable. Error: %d", LastErr());
            return FALSE;
        }
    }

    return TRUE;
}