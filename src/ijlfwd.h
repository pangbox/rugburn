#ifndef IJL15_H
#define IJL15_H

#include "common.h"

typedef int(STDCALL *PFNIJLGETLIBVERSIONPROC)();
typedef int(STDCALL *PFNIJLINITPROC         )(int);
typedef int(STDCALL *PFNIJLFREEPROC         )(int);
typedef int(STDCALL *PFNIJLREADPROC         )(int, int);
typedef int(STDCALL *PFNIJLWRITEPROC        )(int, int);
typedef int(STDCALL *PFNIJLERRORSTRPROC     )(int);

VOID InitIJL15();

#endif
