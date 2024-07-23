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

/*
 * x86 length disassembler, adapted from
 * https://github.com/Nomade040/length-disassembler
 * originally licensed under the Unlicense:
 * https://unlicense.org/
 */

#include "ld.h"

#define R (*b >> 4)
#define C (*b & 0xF)

static const BYTE bPrefixes[] = {0xF0, 0xF2, 0xF3, 0x2E, 0x36, 0x3E, 0x26, 0x64, 0x65, 0x66, 0x67};
static const BYTE bOp1ModRM[] = {0x62, 0x63, 0x69, 0x6B, 0xC0, 0xC1, 0xC4, 0xC5, 0xC6,
                                 0xC7, 0xD0, 0xD1, 0xD2, 0xD3, 0xF6, 0xF7, 0xFE, 0xFF};
static const BYTE bOp1Imm8[] = {0x6A, 0x6B, 0x80, 0x82, 0x83, 0xA8, 0xC0,
                                0xC1, 0xC6, 0xCD, 0xD4, 0xD5, 0xEB};
static const BYTE bOp1Imm32[] = {0x68, 0x69, 0x81, 0xA9, 0xC7, 0xE8, 0xE9};
static const BYTE bOp2ModRM[] = {0x0D, 0xA3, 0xA4, 0xA5, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF};

static BOOL FindByte(const BYTE *arr, const unsigned int N, const BYTE x) {
    unsigned int i;
    for (i = 0; i < N; i++) {
        if (arr[i] == x) {
            return TRUE;
        }
    }
    return FALSE;
}

static void ParseModRM(BYTE **b, const BOOL addressPrefix) {
    BYTE modrm = *++*b;

    if (!addressPrefix || (addressPrefix && **b >= 0x40)) {
        BOOL hasSIB = FALSE;
        if (**b < 0xC0 && (**b & 7) == 4 && !addressPrefix)
            hasSIB = TRUE, (*b)++;
        if (modrm >= 0x40 && modrm <= 0x7F)
            (*b)++;
        else if ((modrm <= 0x3F && (modrm & 7) == 5) || (modrm >= 0x80 && modrm <= 0xBF))
            *b += (addressPrefix) ? 2 : 4;
        else if (hasSIB && (**b & 7) == 5)
            *b += (modrm & 64) ? 1 : 4;
    } else if (addressPrefix && modrm == 0x26)
        *b += 2;
};

unsigned int LenDisasm(LPVOID address) {
    size_t offset = 0;
    BOOL operandPrefix = FALSE, addressPrefix = FALSE, rexW = FALSE;
    BYTE *b = (BYTE *)(address);
    int i;

    for (i = 0; i < 14 && FindByte(bPrefixes, sizeof(bPrefixes), *b); i++, b++) {
        if (*b == 0x66)
            operandPrefix = TRUE;
        else if (*b == 0x67)
            addressPrefix = TRUE;
        else if (R == 4 && C >= 8)
            rexW = TRUE;
    }
    if (*b == 0x0F) {
        b++;
        if (*b == 0x38 || *b == 0x3A) {
            if (*b++ == 0x3A)
                offset++;
            ParseModRM(&b, addressPrefix);
        } else {
            if (R == 8)
                offset += 4;
            else if ((R == 7 && C < 4) || *b == 0xA4 || *b == 0xC2 || (*b > 0xC3 && *b <= 0xC6) ||
                     *b == 0xBA || *b == 0xAC)
                offset++;
            if (FindByte(bOp2ModRM, sizeof(bOp2ModRM), *b) || (R != 3 && R > 0 && R < 7) ||
                *b >= 0xD0 || (R == 7 && C != 7) || R == 9 || R == 0xB || (R == 0xC && C < 8) ||
                (R == 0 && C < 4))
                ParseModRM(&b, addressPrefix);
        }
    } else {
        if ((R == 0xE && C < 8) || (R == 0xB && C < 8) || R == 7 ||
            (R < 4 && (C == 4 || C == 0xC)) || (*b == 0xF6 && !(*(b + 1) & 48)) ||
            FindByte(bOp1Imm8, sizeof(bOp1Imm8), *b))
            offset++;
        else if (*b == 0xC2 || *b == 0xCA)
            offset += 2;
        else if (*b == 0xC8)
            offset += 3;
        else if ((R < 4 && (C == 5 || C == 0xD)) || (R == 0xB && C >= 8) ||
                 (*b == 0xF7 && !(*(b + 1) & 48)) || FindByte(bOp1Imm32, sizeof(bOp1Imm32), *b))
            offset += (rexW) ? 8 : (operandPrefix ? 2 : 4);
        else if (R == 0xA && C < 4)
            offset += (rexW) ? 8 : (addressPrefix ? 2 : 4);
        else if (*b == 0xEA || *b == 0x9A)
            offset += operandPrefix ? 4 : 6;
        if (FindByte(bOp1ModRM, sizeof(bOp1ModRM), *b) ||
            (R < 4 && (C < 4 || (C >= 8 && C < 0xC))) || R == 8 || (R == 0xD && C >= 8))
            ParseModRM(&b, addressPrefix);
    }

    return (size_t)((DWORD_PTR)(++b + offset) - (DWORD_PTR)(address));
}
