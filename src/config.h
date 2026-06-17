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

#pragma once

#include "common.h"
#include "regex.h"

#define MAXURLREWRITES 64
#define MAXPORTREWRITES 16
#define MAXPATCHADDRESS 64

typedef enum {
    TYPE_BYTE,
    TYPE_INT16,
    TYPE_INT,
    TYPE_UINT,
    TYPE_ULONG,
    TYPE_LONG,
    TYPE_STRING,
    TYPE_NOP,
    TYPE_JMP,
    TYPE_JZ,
    TYPE_JE,
    TYPE_JNE,
    TYPE_PATCH
} PATCH_TYPE;

typedef struct _URLREWRITERULE {
    REGEX *from;
    LPCSTR to;
} URLREWRITERULE, *LPURLREWRITERULE;

typedef struct _PORTREWRITERULE {
    WORD fromport;
    WORD toport;
    LPCSTR toaddr;
} PORTREWRITERULE, *LPPORTREWRITERULE;

typedef struct _PATCH_DATA {
    union {
        unsigned char b;
        short i16;
        int i;
        char *str;
        LPSTR patch; // For raw patch data
        struct {
            DWORD target;
        } jmp;
    } val;
} PATCH_DATA;

typedef struct _PATCHADDRESS {
    DWORD addr;
    char *description; // For logging
    PATCH_TYPE type;
    PATCH_DATA data;
    DWORD size;
} PATCHADDRESS, *LPPATCHADDRESS;

typedef struct _RUGBURNCONFIG {
    URLREWRITERULE UrlRewriteRules[MAXURLREWRITES];
    int NumUrlRewriteRules;

    PORTREWRITERULE PortRewriteRules[MAXPORTREWRITES];
    int NumPortRewriteRules;

    PATCHADDRESS PatchAddress[MAXPATCHADDRESS];
    int NumPatchAddress;

    BOOL bBypassSelfSignedCertificate;
} RUGBURNCONFIG, *LPRUGBURNCONFIG;

extern RUGBURNCONFIG Config;

void LoadJsonRugburnConfig();
LPCSTR RewriteURL(LPCSTR url);
BOOL RewriteAddr(LPSOCKADDR_IN addr);
void PatchAddress();
 