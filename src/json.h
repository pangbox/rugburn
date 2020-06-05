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
