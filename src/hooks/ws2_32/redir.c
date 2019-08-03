#include "redir.h"
#include "../../patch.h"

HMODULE hWinsock = NULL;
PFNCONNECTPROC pConnect = NULL;
struct in_addr connectAddr = {127, 0, 0, 1};

/**
 * ConnectHook redirects Winsock TCP sockets to localhost.
 */
int STDCALL ConnectHook(SOCKET s, const struct sockaddr *name, int namelen) {
    struct sockaddr_in name_in = *(struct sockaddr_in*)name;
    struct sockaddr_in override_in = {0};
    char *addr = (char*)&override_in.sin_addr;
    override_in.sin_family = AF_INET;
    override_in.sin_port = name_in.sin_port;
    override_in.sin_addr = connectAddr;
    Log("connect(0x%08x, %d.%d.%d.%d:%d)\r\n", s, addr[0], addr[1], addr[2], addr[3], name_in.sin_port);
    return pConnect(s, (struct sockaddr*)&override_in, sizeof(struct sockaddr_in));
}

VOID InitRedirHook() {
    hWinsock = LoadLib("ws2_32");
    pConnect = HookProc(hWinsock, "connect", ConnectHook);
}
