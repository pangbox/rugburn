#include "hooks.h"

#include "kernel32/inject.h"
#include "msvcr100/msvcr100.h"
#include "user32/window.h"
#include "wininet/netredir.h"
#include "ws2_32/redir.h"

VOID InitHooks() {
    InitInjectHook();
    InitMsvcr100Hook();
    InitWindowHook();
    InitNetRedirHook();
    InitRedirHook();
}
