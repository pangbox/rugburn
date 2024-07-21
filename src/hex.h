#ifndef HEX_H
#define HEX_H

#include "common.h"

LPSTR ToHex(PVOID pData, DWORD dwLen);
DWORD FromHex(LPCSTR pHex, PVOID pData);

DWORD ParseAddress(LPCSTR lpszText);
void ParsePatch(LPCSTR lpszText, LPSTR *pDataOut, DWORD *pSizeOut);

#endif
