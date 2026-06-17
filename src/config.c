/**
 * Copyright 2018-2024 John Chadwick <john@jchw.io>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose
 * with or without fee is hereby granted, provided that the above copyright notice
 * and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */

#include "config.h"
#include "hex.h"
#include "json.h"
#include "patch.h"
#include <ws2tcpip.h>
#include <wspiapi.h>

LPCSTR RugburnConfigFilename = "rugburn.json";
RUGBURNCONFIG Config;
const char ExampleRugburnConfig[] =
    "{\r\n"
    "  \"UrlRewrites\": {\r\n"
    "    \"http://[a-zA-Z0-9:.]+/(.*)\": \"http://localhost:8080/$0\"\r\n"
    "  },\r\n"
    "  \"PortRewrites\": [\r\n"
    "    {\r\n"
    "      \"FromPort\": 10103,\r\n"
    "      \"ToPort\": 10101,\r\n"
    "      \"ToAddr\": \"localhost\"\r\n"
    "    }\r\n"
    "  ]\r\n"
    "}\r\n";

static void ReadJsonUrlRewriteRuleMap(LPSTR *json, LPCSTR key) {
    LPCSTR value = JsonReadString(json);

    if (Config.NumUrlRewriteRules == MAXURLREWRITES) {
        FatalError("Reached maximum number of URL rewrite rules!");
    }

    Config.UrlRewriteRules[Config.NumUrlRewriteRules].from = ReParse(key);
    Config.UrlRewriteRules[Config.NumUrlRewriteRules].to = value;
    Config.NumUrlRewriteRules++;
}

static void ReadJsonPortRewriteRuleMap(LPSTR *json, LPCSTR key) {
    if (!strcmp(key, "FromPort")) {
        Config.PortRewriteRules[Config.NumPortRewriteRules].fromport = JsonReadInteger(json);
    } else if (!strcmp(key, "ToPort")) {
        Config.PortRewriteRules[Config.NumPortRewriteRules].toport = JsonReadInteger(json);
    } else if (!strcmp(key, "ToAddr")) {
        Config.PortRewriteRules[Config.NumPortRewriteRules].toaddr = JsonReadString(json);
    } else {
        FatalError("Unexpected JSON config key in port rewrite rule: '%s'", key);
    }
}

static void ReadJsonPortRewriteRuleArray(LPSTR *json) {
    if (Config.NumPortRewriteRules == MAXURLREWRITES) {
        FatalError("Reached maximum number of URL rewrite rules!");
    }

    JsonReadMap(json, ReadJsonPortRewriteRuleMap);
    Config.NumPortRewriteRules++;
}

static void ReadJsonPatchAddressMap(LPSTR *json, LPCSTR key) {

    // Processa o objeto aninhado (o valor associado à chave do endereço)
    LPPATCHADDRESS p = &Config.PatchAddress[Config.NumPatchAddress];

    if (!strcmp(key, "description")) {
        p->description = DupStr(JsonReadString(json));
    } else if (!strcmp(key, "address")) {
        p->addr = ParseAddress(JsonReadString(json));

    } else if (!strcmp(key, "type")) {
        LPCSTR typeStr = JsonReadString(json);
        if (!strcmp(typeStr, "nop"))
            p->type = TYPE_NOP;
        else if (!strcmp(typeStr, "string"))
            p->type = TYPE_STRING;
        else if (!strcmp(typeStr, "je"))
            p->type = TYPE_JE;
        else if (!strcmp(typeStr, "jne"))
            p->type = TYPE_JNE;
        else if (!strcmp(typeStr, "int"))
            p->type = TYPE_INT;
        else if (!strcmp(typeStr, "uint"))
            p->type = TYPE_UINT;
        else if (!strcmp(typeStr, "jmp"))
            p->type = TYPE_JMP;
        else if (!strcmp(typeStr, "patch"))
            p->type = TYPE_PATCH;
    } else if (!strcmp(key, "value")) {
        if (p->type == TYPE_STRING) {
            p->data.val.str = DupStr(JsonReadString(json));
        }
        if (p->type == TYPE_JE) {
            p->data.val.b = 0x74; // opcode for JE
        }
        if (p->type == TYPE_NOP) {
            p->data.val.b = 0x90; // opcode for JE
        } else if (p->type == TYPE_INT || p->type == TYPE_UINT) {
            p->data.val.i = JsonReadInteger(json);
        } else if (p->type == TYPE_JMP) {
            p->data.val.jmp.target = ParseAddress(JsonReadString(json));
        } else if (p->type == TYPE_PATCH) {
            ParsePatch(JsonReadString(json), &p->data.val.patch, &p->size);
        }
    } else if (!strcmp(key, "length") || !strcmp(key, "size")) {
        p->size = JsonReadInteger(json);
    }
}

static void ReadJsonPatchAddressMapArray(LPSTR *json) {
    if (Config.NumPatchAddress == MAXPATCHADDRESS) {
        FatalError("Reached maximum number of Patch address!");
    }

    JsonReadMap(json, ReadJsonPatchAddressMap);
    Config.NumPatchAddress++;
}

static void ReadJsonBypassSelfSignedCertificate(LPSTR *json, LPCSTR key) {
    LPCSTR value = JsonReadString(json);

    Config.bBypassSelfSignedCertificate = FALSE;

    if (value == NULL || value == "")
        return;

    if (_stricmp(value, "TRUE") == 0)
        Config.bBypassSelfSignedCertificate = TRUE;
}

static void ReadJsonConfigMap(LPSTR *json, LPCSTR key) {
    if (!strcmp(key, "UrlRewrites")) {
        JsonReadMap(json, ReadJsonUrlRewriteRuleMap);
    } else if (!strcmp(key, "PortRewrites")) {
        JsonReadArray(json, ReadJsonPortRewriteRuleArray);
    } else if (!strcmp(key, "PatchAddress")) {
        JsonReadArray(json, ReadJsonPatchAddressMapArray);
    } else if (!strcmp(key, "BypassSelfSignedCertificate")) {
        ReadJsonBypassSelfSignedCertificate(json, key);
    } else {
        FatalError("Unexpected JSON config key '%s'", key);
    }
}

void LoadJsonRugburnConfig() {
    LPSTR json;
    if (!FileExists(RugburnConfigFilename)) {
        Warning("No rugburn.json config file found. An example configuration will be saved.");
        WriteEntireFile(RugburnConfigFilename, ExampleRugburnConfig,
                        sizeof(ExampleRugburnConfig) - 1);
        json = DupStr(ExampleRugburnConfig);
    } else {
        json = ReadEntireFile(RugburnConfigFilename, NULL);
    }
    memset(&Config, 0, sizeof(RUGBURNCONFIG));
    JsonReadMap(&json, ReadJsonConfigMap);
}

LPCSTR RewriteURL(LPCSTR url) {
    LPCSTR result;
    int i;

    for (i = 0; i < Config.NumUrlRewriteRules; i++) {
        result = ReReplace(Config.UrlRewriteRules[i].from, Config.UrlRewriteRules[i].to, url);
        if (result != NULL) {
            return result;
        }
    }
    return NULL;
}

BOOL RewriteAddr(LPSOCKADDR_IN addr) {
    ADDRINFOA hints;
    ADDRINFOA *resolved, *ptr;
    DWORD result;
    int i;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    for (i = 0; i < Config.NumPortRewriteRules; i++) {
        if (addr->sin_port == htons(Config.PortRewriteRules[i].fromport)) {
            result = getaddrinfo(Config.PortRewriteRules[i].toaddr, NULL, &hints, &resolved);
            if (result != 0) {
                Log("Warning: failed to resolve %s (result=%08x)\r\n",
                    Config.PortRewriteRules[i].toaddr, result);
                continue;
            }
            ptr = resolved;
            do {
                if (ptr->ai_family != AF_INET) {
                    Log("Skipping result because it is for a different address family (%i)",
                        ptr->ai_family);
                }
                break;
            } while ((ptr = ptr->ai_next));
            if (!ptr) {
                Log("Warning: no suitable result for %s.", Config.PortRewriteRules[i].toaddr);
                continue;
            }
            memcpy(addr, resolved->ai_addr, sizeof(struct sockaddr_in));
            addr->sin_port = htons(Config.PortRewriteRules[i].toport);
            freeaddrinfo(resolved);
            return TRUE;
        }
    }

    return FALSE;
}

void PatchAddress() {
    for (int i = 0; i < Config.NumPatchAddress; i++) {
        PATCHADDRESS *p = &Config.PatchAddress[i];

        // Verificação básica de segurança
        if (p->addr == 0)
            continue;

        if (p->description) {
            unsigned char *oldBuffer = (unsigned char *)calloc(p->size, 1);
            if (oldBuffer) {
                Read(p->addr, oldBuffer, p->size);

                if (p->description) {
                    if (p->type == TYPE_STRING) {
                        Log("[Rugburn] PATCH[%s] | OLD: '%s' -> NEW: '%s'\r\n", p->description,
                            oldBuffer, p->data.val.str);
                    } else if (p->type == TYPE_UINT || p->type == TYPE_INT) {
                        unsigned int oldVal = *(unsigned int *)oldBuffer;
                        Log("[Rugburn] PATCH[%s] | OLD: %u -> NEW: %u\r\n", p->description, oldVal,
                            p->data.val.i);
                    }
                }
                free(oldBuffer);
            }
        }

        switch (p->type) {

        case TYPE_NOP: {
            unsigned char *nops = (unsigned char *)malloc(p->size);
            memset(nops, 0x90, p->size);
            Patch(p->addr, nops, p->size);
            free(nops);
            break;
        }
        case TYPE_BYTE:
            Patch(p->addr, &p->data.val.b, 1);
            break;
        case TYPE_INT16:
            Patch(p->addr, &p->data.val.i16, 2);
            break;
        case TYPE_INT:
        case TYPE_UINT:
            Patch(p->addr, &p->data.val.i, 4);
            break;
        case TYPE_ULONG:
        case TYPE_LONG:
            Patch(p->addr, &p->data.val.i, 8);
            break;
        case TYPE_STRING: {
            unsigned char *buffer = (unsigned char *)calloc(p->size, 1);
            if (buffer) {
                size_t strLen = strlen(p->data.val.str);
                size_t copyLen = (strLen < p->size) ? strLen : p->size;
                memcpy(buffer, p->data.val.str, copyLen);
                Patch(p->addr, buffer, p->size);
                free(buffer);
            }
            break;
        }
        case TYPE_JMP: {
            unsigned char jmpCode = 0xE9;
            DWORD offset = p->data.val.jmp.target - (p->addr + 5);
            Patch(p->addr, &jmpCode, 1);
            Patch(p->addr + 1, &offset, 4);
            break;
        }
        case TYPE_JZ: {
            unsigned short jzCode = 0x840F; // Opcode 0F 84
            DWORD offset = p->data.val.jmp.target - (p->addr + 6);
            Patch(p->addr, &jzCode, 2);
            Patch(p->addr + 2, &offset, 4);
            break;
        }
        case TYPE_JE: {
            unsigned char je = 0x74;
            Patch(p->addr, &je, 1);
            break;
        }
        case TYPE_JNE: {
            unsigned char jne = 0x75;
            Patch(p->addr, &jne, 1);
            break;
        }
        case TYPE_PATCH: {
            Patch(p->addr, p->data.val.patch, p->size);
            break;
        }
        }
    }
}