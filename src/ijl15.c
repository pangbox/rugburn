#include "../third_party/ijl/ijl.h"
#include <windows.h>

const IJLibVersion *__stdcall ijlGetLibVersionWrapper() { return ijlGetLibVersion(); }

IJLERR __stdcall ijlInitWrapper(JPEG_CORE_PROPERTIES *jcprops) { return ijlInit(jcprops); }

IJLERR __stdcall ijlFreeWrapper(JPEG_CORE_PROPERTIES *jcprops) { return ijlFree(jcprops); }

IJLERR __stdcall ijlReadWrapper(JPEG_CORE_PROPERTIES *jcprops, IJLIOTYPE iotype) {
    return ijlRead(jcprops, iotype);
}

IJLERR __stdcall ijlWriteWrapper(JPEG_CORE_PROPERTIES *jcprops, IJLIOTYPE iotype) {
    return ijlWrite(jcprops, iotype);
}

const char *__stdcall ijlErrorStrWrapper(IJLERR code) { return ijlErrorStr(code); }
