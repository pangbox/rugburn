#include "hex.h"

static BOOL ParseHex(CHAR ch, LPDWORD pOutValue) {
    if (ch >= '0' && ch <= '9') {
        *pOutValue = ch - '0';
    } else if (ch >= 'a' && ch <= 'f') {
        *pOutValue = ch - 'a' + 10;
    } else if (ch >= 'A' && ch <= 'F') {
        *pOutValue = ch - 'A' + 10;
    } else {
        return FALSE;
    }
    return TRUE;
}

DWORD ParseAddress(LPCSTR lpszText) {
    LPCSTR pos = lpszText;
    DWORD hexDigitVal;
    DWORD result = 0;

    if (lpszText == NULL) {
        FatalError("ParseAddress: Received unexpected NULL string");
    }

    if (lpszText[0] == '\0') {
        return 0;
    }

    if (pos[0] != '0' || pos[1] != 'x' || pos[2] == 0) {
        FatalError("ParseAddress: Invalid input, expected 0x-prefixed hex number");
    }

    pos += 2;

    while (ParseHex(*pos, &hexDigitVal) == TRUE) {
        result <<= 4;
        result |= hexDigitVal;
        pos++;
    }

    if (*pos) {
        FatalError("ParseAddress: Unexpected text remaining after address: %s", pos);
    }

    return result;
}

void ParsePatch(LPCSTR lpszText, LPSTR *pDataOut, DWORD *pSizeOut) {
    DWORD hexDigitVal;
    DWORD hexOctetVal;
    LPCSTR inPos;
    LPSTR outPos;
    *pSizeOut = 0;

    // Calculate length
    inPos = lpszText;
    while (*inPos) {
        switch (*inPos++) {
            case '\\':
                switch (*inPos++) {
                    case 'x':
                        if (ParseHex(*inPos++, &hexDigitVal) == FALSE) {
                            FatalError("ParsePatch: Bad hex escape near %s", inPos);
                        }
                        if (ParseHex(*inPos++, &hexDigitVal) == FALSE) {
                            FatalError("ParsePatch: Bad hex escape near %s", inPos);
                        }
                        (*pSizeOut)++;
                        break;
                    case '\\':
                        (*pSizeOut)++;
                        break;
                    case '\0':
                        FatalError("ParsePatch: Unexpected end of string in escape code");
                        break;
                    default:
                        FatalError("ParsePatch: Unknown escape code: %s", inPos);
                }
                break;
            default:
                (*pSizeOut)++;
        }
    }

    // Allocate memory
    *pDataOut = AllocMem(*pSizeOut);

    // Parse
    inPos = lpszText;
    outPos = *pDataOut;
    while (*inPos) {
        switch (*inPos) {
            case '\\':
                inPos++;
                switch (*inPos++) {
                    case 'x':
                        if (!ParseHex(*inPos++, &hexDigitVal)) {
                            FatalError("ParsePatch: Bad hex escape near %s", inPos);
                        }
                        hexOctetVal = hexDigitVal << 4;
                        if (!ParseHex(*inPos++, &hexDigitVal)) {
                            FatalError("ParsePatch: Bad hex escape near %s", inPos);
                        }
                        hexOctetVal |= hexDigitVal;
                        *outPos++ = (CHAR)(BYTE)hexOctetVal;
                        break;
                    case '\\':
                        *outPos++ = '\\';
                        break;
                    case '\0':
                        FatalError("ParsePatch: Unexpected end of string in escape code");
                        break;
                    default:
                        FatalError("ParsePatch: Unknown escape code: %s", inPos);
                }
                break;
            default:
                *(outPos++) = *(inPos++);
        }
    }
}
