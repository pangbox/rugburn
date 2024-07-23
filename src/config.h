#ifndef CONFIG_H
#define CONFIG_H

#include "common.h"
#include "regex.h"

#define MAXURLREWRITES 64
#define MAXPORTREWRITES 16
#define MAXPATCHADDRESS 64

typedef struct _URLREWRITERULE {
    REGEX *from;
    LPCSTR to;
} URLREWRITERULE, *LPURLREWRITERULE;

typedef struct _PORTREWRITERULE {
    WORD fromport;
    WORD toport;
    LPCSTR toaddr;
} PORTREWRITERULE, *LPPORTREWRITERULE;

typedef struct _PATCHADDRESS {
    DWORD addr;
    LPSTR patch;
    DWORD patchLen;
} PATCHADDRESS, *LPPATCHADDRESS;

typedef struct _RUGBURNCONFIG {
    URLREWRITERULE UrlRewriteRules[MAXURLREWRITES];
    int NumUrlRewriteRules;

    PORTREWRITERULE PortRewriteRules[MAXPORTREWRITES];
    int NumPortRewriteRules;

    PATCHADDRESS PatchAddress[MAXPATCHADDRESS];
    int NumPatchAddress;
} RUGBURNCONFIG, *LPRUGBURNCONFIG;

extern RUGBURNCONFIG Config;

void LoadJsonRugburnConfig();
LPCSTR RewriteURL(LPCSTR url);
BOOL RewriteAddr(LPSOCKADDR_IN addr);
void PatchAddress();

#endif
