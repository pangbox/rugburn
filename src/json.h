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

#ifndef JSON_H
#define JSON_H

#include "common.h"

typedef enum _JSONTOKENTYPE {
    JSON_TOK_ERROR,
    JSON_TOK_LBRACE,
    JSON_TOK_RBRACE,
    JSON_TOK_LBRACKET,
    JSON_TOK_RBRACKET,
    JSON_TOK_COMMA,
    JSON_TOK_COLON,
    JSON_TOK_STRING,
    JSON_TOK_NUMBER,
    JSON_TOK_TRUE,
    JSON_TOK_FALSE,
    JSON_TOK_NULL,
} JSONTOKENTYPE;

typedef struct {
    JSONTOKENTYPE token_type;
    LPCSTR string_val;
    int int_val;
} JSONTOKEN, *LPJSONTOKEN;

typedef void (*LFNREADMAPVALUECB)(LPSTR *json, LPCSTR key);
typedef void (*LFNREADARRAYVALUECB)(LPSTR *json);

void JsonNextToken(LPSTR *json, LPJSONTOKEN token);
void JsonExpectToken(LPSTR *json, JSONTOKENTYPE type);
LPCSTR JsonReadString(LPSTR *json);
int JsonReadInteger(LPSTR *json);
BOOL JsonReadBool(LPSTR *json);
void JsonReadMap(LPSTR *json, LFNREADMAPVALUECB valuefn);
void JsonReadArray(LPSTR *json, LFNREADARRAYVALUECB valuefn);

#endif
