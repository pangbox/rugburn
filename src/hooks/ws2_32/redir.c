#include "redir.h"
#include "../../config.h"
#include "../../patch.h"

static HMODULE hWinsockModule = NULL;
static PFNCONNECTPROC pConnect = NULL;

/**
 * ConnectHook redirects Winsock TCP sockets to localhost.
 */
static int STDCALL ConnectHook(SOCKET s, const struct sockaddr *name, int namelen) {
    struct sockaddr_in name_in = *(struct sockaddr_in *)name;
    struct sockaddr_in override_in;
    char *oldaddr = (char *)&name_in.sin_addr;
    char *newaddr = (char *)&override_in.sin_addr;
    memcpy(&override_in, &name_in, sizeof(struct sockaddr_in));

    if (RewriteAddr(&override_in)) {
        Log("connect(0x%08x, %d.%d.%d.%d:%d => %d.%d.%d.%d:%d)\r\n", s, (unsigned char)oldaddr[0],
            (unsigned char)oldaddr[1], (unsigned char)oldaddr[2], (unsigned char)oldaddr[3],
            htons(name_in.sin_port), (unsigned char)newaddr[0], (unsigned char)newaddr[1],
            (unsigned char)newaddr[2], (unsigned char)newaddr[3], htons(override_in.sin_port));
    } else {
        Log("connect(0x%08x, %d.%d.%d.%d:%d) // (no rewrite rules matched)\r\n", s,
            (unsigned char)newaddr[0], (unsigned char)newaddr[1], (unsigned char)newaddr[2],
            (unsigned char)newaddr[3], htons(override_in.sin_port));
    }

    return pConnect(s, (struct sockaddr *)&override_in, sizeof(struct sockaddr_in));
}

VOID InitRedirHook() {
    hWinsockModule = LoadLib("ws2_32");
    pConnect = HookProc(hWinsockModule, "connect", ConnectHook);
}
