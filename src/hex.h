#ifndef HEX_H
#define HEX_H

#include "common.h"

DWORD ParseAddress(LPCSTR lpszText);
void ParsePatch(LPCSTR lpszText, LPSTR *pDataOut, DWORD *pSizeOut);

#endif
