#include "hooks.h"

#include "kernel32/inject.h"
#include "msvcr100/msvcr100.h"
#include "user32/window.h"
#include "wininet/netredir.h"
#include "ws2_32/redir.h"
#include "comctl32/dynamic_patch.h"

VOID InitHooks() {
    InitInjectHook();
    InitMsvcrHook();
    InitWindowHook();
    InitNetRedirHook();
    InitRedirHook();
    InitComCtl32Hook();
}
