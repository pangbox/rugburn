#ifndef BOOTSTRAP_H
#define BOOTSTRAP_H

#include "common.h"

extern PFNLOADLIBRARYAPROC pLoadLibraryA;
extern PFNGETPROCADDRESSPROC pGetProcAddress;

VOID BootstrapPEB();
VOID BootstrapSlipstream(DWORD dwModuleAddress);

#endif
