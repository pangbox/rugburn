/*
 * Forwards ijl15 API calls down to ijl15_real, allowing this library to be
 * a drop-in replacement for ijl15.
 *
 * TODO(john): Instead, we should probably just patch our code into the 
 * existing ijl15 DLL... but that takes a bit more work.
 */

#include "ijlfwd.h"

HMODULE                 hIJL15Module      = NULL;
PFNIJLGETLIBVERSIONPROC pIJLGetLibVersion = NULL;
PFNIJLINITPROC          pIJLInit          = NULL;
PFNIJLFREEPROC          pIJLFree          = NULL;
PFNIJLREADPROC          pIJLRead          = NULL;
PFNIJLWRITEPROC         pIJLWrite         = NULL;
PFNIJLERRORSTRPROC      pIJLErrorStr      = NULL;

int STDCALL ijlGetLibVersion()             { return pIJLGetLibVersion();     }
int STDCALL ijlInit         (int a)        { return pIJLInit         (a);    }
int STDCALL ijlFree         (int a)        { return pIJLFree         (a);    }
int STDCALL ijlRead         (int a, int b) { return pIJLRead         (a, b); }
int STDCALL ijlWrite        (int a, int b) { return pIJLWrite        (a, b); }
int STDCALL ijlErrorStr     (int err)      { return pIJLErrorStr     (err);  }

VOID InitIJL15() {
    if (hIJL15Module != NULL) {
        FatalError("Error: ijl15 is already initialized!");
    }

    hIJL15Module      = LoadLib("ijl15_real");
    pIJLGetLibVersion = (PFNIJLGETLIBVERSIONPROC)GetProc(hIJL15Module, "ijlGetLibVersion");
    pIJLInit          = (PFNIJLINITPROC         )GetProc(hIJL15Module, "ijlInit");
    pIJLFree          = (PFNIJLFREEPROC         )GetProc(hIJL15Module, "ijlFree");
    pIJLRead          = (PFNIJLREADPROC         )GetProc(hIJL15Module, "ijlRead");
    pIJLWrite         = (PFNIJLWRITEPROC        )GetProc(hIJL15Module, "ijlWrite");
    pIJLErrorStr      = (PFNIJLERRORSTRPROC     )GetProc(hIJL15Module, "ijlErrorStr");
}
