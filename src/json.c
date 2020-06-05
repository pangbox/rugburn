/*
 * Simple JSON parsing utilities for C. Supports most of JSON, but not
 * floating point numbers.
 */

#include "json.h"

BOOL JsonIsSpace(CHAR ch) {
    return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == '\v';
}

LPCSTR JsonTokenName(JSONTOKENTYPE type) {
    switch(type) {
    case JSON_TOK_ERROR:
        return "ERROR";
    case JSON_TOK_LBRACE:
        return "{";
    case JSON_TOK_RBRACE:
        return "}";
    case JSON_TOK_LBRACKET:
        return "[";
    case JSON_TOK_RBRACKET:
        return "]";
    case JSON_TOK_COMMA:
        return ",";
    case JSON_TOK_COLON:
        return ":";
    case JSON_TOK_STRING:
        return "string";
    case JSON_TOK_NUMBER:
        return "number";
    case JSON_TOK_TRUE:
        return "true";
    case JSON_TOK_FALSE:
        return "false";
    case JSON_TOK_NULL:
        return "null";
    }
    return "(unknown)";
}

void JsonConsumeWhitespace(LPSTR *json) {
    while (JsonIsSpace(**json)) (*json)++;
}

void JsonConsumeIntegerToken(LPSTR *json, LPJSONTOKEN token) {
    int value = 0;
    int sign = 1;

    // Consume sign.
    if (**json == '-') {
        sign = -sign;
        (*json)++;
    }

    // Consume digits.
    while (1) {
        switch(**json) {
        case '\0':
            FatalError("Parsing JSON: Unexpected end of file.");
            return;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            value = value * 10 + *(*json)++ - '0';
            break;
        default:
            // Apply sign.
            if (sign == -1) {
                value = -value;
            }
            token->token_type = JSON_TOK_NUMBER;
            token->int_val = value;
            return;
        }
    }
}

void JsonConsumeSymbolToken(LPSTR *json, LPJSONTOKEN token) {
    switch(*(*json)++) {
    case '{': token->token_type = JSON_TOK_LBRACE; return;
    case '}': token->token_type = JSON_TOK_RBRACE; return;
    case '[': token->token_type = JSON_TOK_LBRACKET; return;
    case ']': token->token_type = JSON_TOK_RBRACKET; return;
    case ',': token->token_type = JSON_TOK_COMMA; return;
    case ':': token->token_type = JSON_TOK_COLON; return;
    default: FatalError("Programming error in JSON parser: Unexpected symbol token."); break;
    }
}

void JsonConsumeStringToken(LPSTR *json, LPJSONTOKEN token) {
    LPCSTR strptr;
    LPSTR wptr;
    CHAR ch;

    if (**json == '"') {
        (*json)++;
    } else {
        FatalError("Parsing JSON: expected '\"', got '%c'.", **json);
    }

    strptr = *json;
    wptr = *json;

    while(1) {
        ch = *(*json)++;
        switch(ch) {
        case '\0':
            FatalError("Parsing JSON: Unexpected end of file.");
            return;
        case '\\':
            ch = *(*json)++;
            switch(ch) {
            case '\0':
                FatalError("Parsing JSON: Unexpected end of file.");
                return;
            case '\"':
                *wptr++ = '\"';
                break;
            case '\\':
                *wptr++ = '\\';
                break;
            case '/':
                *wptr++ = '/';
                break;
            case 'b':
                *wptr++ = '\b';
                break;
            case 'f':
                *wptr++ = '\f';
                break;
            case 'n':
                *wptr++ = '\n';
                break;
            case 'r':
                *wptr++ = '\r';
                break;
            case 't':
                *wptr++ = '\t';
                break;
            }
            break;
        case '"':
            *wptr++ = '\0';
            token->token_type = JSON_TOK_STRING;
            token->string_val = strptr;
            return;
        default: 
            *wptr++ = ch;
            break;
        }
    }
}

void JsonConsumeKeywordToken(LPSTR *json, LPJSONTOKEN token) {
    if (memcmp(*json, "true", 4) == 0) {
        *json += 4;
        token->token_type = JSON_TOK_TRUE;
        return;
    }
    if (memcmp(*json, "false", 5) == 0) {
        *json += 5;
        token->token_type = JSON_TOK_FALSE;
        return;
    }
    if (memcmp(*json, "null", 4) == 0) {
        *json += 4;
        token->token_type = JSON_TOK_NULL;
        return;
    }
    FatalError("Parsing JSON: Unexpected character '%c'", **json);
}

void JsonNextToken(LPSTR *json, LPJSONTOKEN token) {
    JsonConsumeWhitespace(json);
    switch (**json) {
    case '\0':
        FatalError("Parsing JSON: Unexpected end of file.");
        return;
    case '{':
    case '}':
    case '[':
    case ']':
    case ',':
    case ':':
        JsonConsumeSymbolToken(json, token);
        return;
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        JsonConsumeIntegerToken(json, token);
        return;
    case '"':
        JsonConsumeStringToken(json, token);
        return;
    default:
        JsonConsumeKeywordToken(json, token);
        return;
    }
}

void JsonExpectToken(LPSTR *json, JSONTOKENTYPE type) {
    JSONTOKEN token;
    JsonNextToken(json, &token);
    if (token.token_type != type) {
        FatalError("Parsing JSON: Expected '%s' token, got '%s'", JsonTokenName(type), JsonTokenName(token.token_type));
    }
}

LPCSTR JsonReadString(LPSTR *json) {
    JSONTOKEN token;

    JsonNextToken(json, &token);
    if (token.token_type != JSON_TOK_STRING) {
        FatalError("Parsing JSON: Expected string token, got '%s'", JsonTokenName(token.token_type));
    }

    return token.string_val;
}

int JsonReadInteger(LPSTR *json) {
    JSONTOKEN token;

    JsonNextToken(json, &token);
    if (token.token_type != JSON_TOK_NUMBER) {
        FatalError("Parsing JSON: Expected integer token, got '%s'", JsonTokenName(token.token_type));
    }

    return token.int_val;
}

BOOL JsonReadBool(LPSTR *json) {
    JSONTOKEN token;

    JsonNextToken(json, &token);
    if (token.token_type != JSON_TOK_TRUE && token.token_type != JSON_TOK_FALSE) {
        FatalError("Parsing JSON: Expected boolean token, got '%s'", JsonTokenName(token.token_type));
    }

    return token.token_type == JSON_TOK_TRUE ? TRUE : FALSE;
}

BOOL JsonReadMapSeparator(LPSTR *json) {
    JSONTOKEN token;

    JsonNextToken(json, &token);
    if (token.token_type != JSON_TOK_COMMA && token.token_type != JSON_TOK_RBRACE) {
        FatalError("Parsing JSON: Expected ',' or '}', got '%s'", JsonTokenName(token.token_type));
    }

    return token.token_type == JSON_TOK_COMMA ? TRUE : FALSE;
}

void JsonReadMap(LPSTR *json, LFNREADMAPVALUECB valuefn) {
    LPCSTR key;
    JsonExpectToken(json, JSON_TOK_LBRACE);
    do {
        key = JsonReadString(json);
        JsonExpectToken(json, JSON_TOK_COLON);
        valuefn(json, key);
    } while(JsonReadMapSeparator(json));
}

BOOL JsonReadArraySeparator(LPSTR *json) {
    JSONTOKEN token;

    JsonNextToken(json, &token);
    if (token.token_type != JSON_TOK_COMMA && token.token_type != JSON_TOK_RBRACKET) {
        FatalError("Parsing JSON: Expected ',' or ']', got '%s'", JsonTokenName(token.token_type));
    }

    return token.token_type == JSON_TOK_COMMA ? TRUE : FALSE;
}

void JsonReadArray(LPSTR *json, LFNREADARRAYVALUECB valuefn) {
    JsonExpectToken(json, JSON_TOK_LBRACKET);
    do { valuefn(json); } while(JsonReadArraySeparator(json));
}
