#ifndef CONFIG_H
#define CONFIG_H

#include "common.h"
#include "regex.h"

#define MAXURLREWRITES 64
#define MAXPORTREWRITES 16

typedef struct _URLREWRITERULE {
    REGEX *from;
    LPCSTR to;
} URLREWRITERULE, *LPURLREWRITERULE;

typedef struct _PORTREWRITERULE {
    WORD fromport;
    WORD toport;
    LPCSTR toaddr;
} PORTREWRITERULE, *LPPORTREWRITERULE;

typedef struct _RUGBURNCONFIG {
    URLREWRITERULE UrlRewriteRules[MAXURLREWRITES];
    int NumUrlRewriteRules;

    PORTREWRITERULE PortRewriteRules[MAXPORTREWRITES];
    int NumPortRewriteRules;
} RUGBURNCONFIG, *LPRUGBURNCONFIG;

extern RUGBURNCONFIG Config;

void LoadJsonRugburnConfig();
LPCSTR RewriteURL(LPCSTR url);
BOOL RewriteAddr(LPSOCKADDR_IN addr);

#endif
