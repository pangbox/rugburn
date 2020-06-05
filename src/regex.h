#ifndef RE_H
#define RE_H

#include <windows.h>

typedef struct _REGEX REGEX;

REGEX *ReParse(LPCSTR pattern);
BOOL ReMatch(REGEX *pattern, LPCSTR text);
LPSTR ReReplace(REGEX *find, LPCSTR replace, LPCSTR text);

int ReGetNumCaptures(REGEX *pattern);
int ReGetCaptureLen(REGEX *pattern, int i);
void ReGetCaptureData(REGEX *pattern, int i, LPSTR buffer);

#endif
