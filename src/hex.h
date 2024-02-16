#ifndef HEX_H
#define HEX_H

#include "common.h"

DWORD ReadDword(LPCSTR _text);
void TranslateHexInText(LPCSTR _text, LPSTR _result, int _size_result, int* _size_out);

#endif