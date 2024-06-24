#include "dynamic_patch.h"
#include "../../config.h"
#include "../../hooks/projectg/us852/ranking.h"
#include "../../patch.h"

static HMODULE hComCtl32 = NULL;
static PFNINITCOMMONCONTROLSEXPROC pInitCommonControlsEx = NULL;

BOOL compare_virtual_memory(DWORD _dwAddress, DWORD _value) {
    MEMORY_BASIC_INFORMATION mbi;

    if (pVirtualQuery((void *)_dwAddress, &mbi, sizeof(mbi)) == 0) {
        Log("[compare_virtual_memory] Address 0x%08lX failed in VirtualQuery, ErrorCode: %08lX\r\n",
            _dwAddress, pGetLastError());
        return FALSE;
    }

    if (mbi.Type != MEM_IMAGE) {
        Log("[compare_virtual_memory] 0x%08lX is not image memory.\r\n", _dwAddress);
        return FALSE;
    }

    if (*(DWORD *)_dwAddress != _value)
        return FALSE;

    return TRUE;
}

/**
 * Implements the GameGuard patches for Pangya US.
 */
void PatchGG_US() {
    if (compare_virtual_memory(0x00A495E0, 0x8F143D83)) {
        Patch((LPVOID)0x00A495E0, "\xC3\x90\x90\x90\x90\x90\x90", 7);
        Patch((LPVOID)0x00A49670, "\xC3\x90\x90\x90\x90\x90\x90", 7);
        Patch((LPVOID)0x00A49690, "\xC3\x90\x90\x90\x90\x90\x90", 7);
        Patch((LPVOID)0x00A496B0, "\x90\x90\x90\x90\x90\x90\x90", 7);
        Patch((LPVOID)0x00A496E0, "\xC3", 1);
        Patch((LPVOID)0x00A49840, "\xC3", 1);
        Log("Patched GG check routines (US 852)\r\n");

        Patch((LPVOID)0x00A6ECC9, "\x30\xC0", 2);
        Log("Patched Cookie Point Item (US 852)\r\n");

        Patch((LPVOID)0x005FB990,
              "\x80\xB9\x40\x02\x00\x00\x00\x0F\x85\x0D\x00\x00\x00\x8B\x89\x8C\x01\x00\x00\x8B"
              "\x01\x8B\x50\x4C\xFF\xD2\xC2\x04\x00",
              29);
        Log("Patched Cookie Btn in onCallback that's disabled (US 852)\r\n");

        Patch((LPVOID)0x005FB9AD,
              "\xB3\x01\x31\xFF\x90\x53\xBA\xB4\x6A\xCE\x00\xE9\xC9\x31\x00\x00", 16);
        Patch((LPVOID)0x005FEB7E, "\xE9\x2A\xCE\xFF\xFF", 5);
        Patch((LPVOID)0x005FEB8D, "\x53", 1);
        Patch((LPVOID)0x005FEB9A, "\x53", 1);
        Patch((LPVOID)0x005FB9BD,
              "\x6A\x01\xBA\xB4\x6A\xCE\x00\x8B\xCE\xE8\xF5\x2C\x00\x00\x6A\x01\xBA\xB0\x69\xCE"
              "\x00\xE9\x29\x32\x00\x00",
              26);
        Patch((LPVOID)0x005FEBF9, "\xE9\xBF\xCD\xFF\xFF", 5);
        Log("Patched Btn Cookie, Gacha and Scratch disabled (US 852)\r\n");

        Patch((LPVOID)0x008BC729, "\x01", 1);
        Patch((LPVOID)0x008C1495, "\xEB\x0C", 2);
        Patch((LPVOID)0x008C14A3, "\xE8\xF8\xB2\xFF\xFF\x88\x86\xE4\x00\x00\x00\x5E\xC3", 13);
        Log("Patched Btn Change Nickname disabled (US 852)\r\n");

        InitUS852RankingHook();
        Log("Patched Ranking System disabled (US 852)\r\n");
    } else if (compare_virtual_memory(0x00A49580, 0x8F143D83)) {
        Patch((LPVOID)0x00A49580, "\xC3\x90\x90\x90\x90\x90\x90", 7);
        Patch((LPVOID)0x00A49670, "\xC3\x90\x90\x90\x90\x90\x90", 7);
        Patch((LPVOID)0x00A49690, "\xC3\x90\x90\x90\x90\x90\x90", 7);
        Patch((LPVOID)0x00A496B0, "\x90\x90\x90\x90\x90\x90\x90", 7);
        Patch((LPVOID)0x00A496E0, "\xC3", 1);
        Patch((LPVOID)0x00A49840, "\xC3", 1);
        Log("Patched GG check routines (US 824)\r\n");
    } else if (compare_virtual_memory(0x006E469B, 0xFF6450E8)) {
        Patch((LPVOID)0x006E469B, "\xE9\x00\x00\x00\x00", 5);
        Log("Patched GG check routines (US 431)\r\n");
    } else if (compare_virtual_memory(0x0066DB32, 0xFF6D89E8)) {
        Patch((LPVOID)0x0066DB32, "\xE9\x00\x00\x00\x00", 5);
        Log("Patched GG check routines (Albatross S2 323a)\r\n");
    }
}

/**
 * Implements the GameGuard patches for Pangya JP.
 */
void PatchGG_JP() {
    if (compare_virtual_memory(0x00A5CD10, 0x1BA43D83)) {
        Patch((LPVOID)0x00A5CD10, "\xC3\x90\x90\x90\x90\x90\x90", 7);
        Patch((LPVOID)0x00A5CDA0, "\xC3\x90\x90\x90\x90\x90\x90", 7);
        Patch((LPVOID)0x00A5CDC0, "\xC3\x90\x90\x90\x90\x90\x90", 7);
        Patch((LPVOID)0x00A5CDE0, "\x90\x90\x90\x90\x90\x90\x90", 7);
        Patch((LPVOID)0x00A5CE10, "\xC3", 1);
        Patch((LPVOID)0x00A5CE40, "\xC3", 1);
        Log("Patched GG check routines (JP 972)\r\n");
    } else if (compare_virtual_memory(0x00A5CF80, 0x1BA43D83)) {
        Patch((LPVOID)0x00A5CF80, "\xC3\x90\x90\x90\x90\x90\x90", 7);
        Patch((LPVOID)0x00A5C010, "\xC3\x90\x90\x90\x90\x90\x90", 7);
        Patch((LPVOID)0x00A5C030, "\xC3\x90\x90\x90\x90\x90\x90", 7);
        Patch((LPVOID)0x00A5C050, "\x90\x90\x90\x90\x90\x90\x90", 7);
        Patch((LPVOID)0x00A5CE80, "\xC3", 1);
        Patch((LPVOID)0x00A5CEB0, "\xC3", 1);
        Log("Patched GG check routines (JP 974)\r\n");
    } else if (compare_virtual_memory(0x00A5CF80, 0x1C143D83)) {
        Patch((LPVOID)0x00A5CF80, "\xC3\x90\x90\x90\x90\x90\x90", 7);
        Patch((LPVOID)0x00A5C010, "\xC3\x90\x90\x90\x90\x90\x90", 7);
        Patch((LPVOID)0x00A5C030, "\xC3\x90\x90\x90\x90\x90\x90", 7);
        Patch((LPVOID)0x00A5C050, "\x90\x90\x90\x90\x90\x90\x90", 7);
        Patch((LPVOID)0x00A5CE80, "\xC3", 1);
        Patch((LPVOID)0x00A5CEB0, "\xC3", 1);
        Log("Patched GG check routines (JP 983)\r\n");
    }
}

void PatchGG_TW() {
    if (compare_virtual_memory(0x00643743, 0xFF6B98E8)) {
        Patch((LPVOID)0x00643743, "\xE9\x00\x00\x00\x00", 5);
        Log("Patched GG check routines (TW S2 3.00a)\r\n");
    } else if (compare_virtual_memory(0x006690D1, 0xFF7BEAE8)) {
        Patch((LPVOID)0x006690D1, "\xE9\x00\x00\x00\x00", 5);
        Log("Patched GG check routines (TW S3 4.00a)\r\n");
    }
}

void PatchHS_ID() {
    if (compare_virtual_memory(0x005FC66D, 0xFF071EE8)) {
        Patch((LPVOID)0x005FC66D, "\xE9\x00\x00\x00\x00", 5);
        Log("Patched HSHIELD check routines (INA S1 2.12a)\r\n");
    }
}

void PatchHS_BR() {
    if (compare_virtual_memory(0x005FE1B3, 0xFF8858E8)) {
        Patch((LPVOID)0x005FE1B3, "\xE9\x00\x00\x00\x00", 5);
        Log("Patched HSHIELD check routines (BR S1 2.15a)\r\n");
    } else if (compare_virtual_memory(0x0065B852, 0xFF6269E8)) {
        Patch((LPVOID)0x0065B852, "\xE9\x00\x00\x00\x00", 5);
        Log("Patched HSHIELD check routines (BR S2 3.05a)\r\n");
    }
}

void PatchGG_KR() {
    if (compare_virtual_memory(0x00634EAD, 0xFF178EE8)) {
        Patch((LPVOID)0x00634EAD, "\xE9\x00\x00\x00\x00", 5);
        Log("Patched GG check routines (KR S2 3.26a)\r\n");
    }
}

void PatchGG_TH() {
    if (compare_virtual_memory(0x0076F4F4, 0xFEBED7E8)) {
        Patch((LPVOID)0x0076F4F4, "\xE9\x00\x00\x00\x00", 5);
        Log("Patched GG check routines (TH S4 580)\r\n");
    } else if (compare_virtual_memory(0x00A6B375, 0xFE9A16E8)) {
        Patch((LPVOID)0x00A6B375, "\xE9\x00\x00\x00\x00", 5);
        Log("Patched GG check routines (TH S9 829c)\r\n");
    }
}

void PatchGG_SEA() {
    if (compare_virtual_memory(0x00611CF8, 0xFF86D3E8)) {
        Patch((LPVOID)0x00611CF8, "\xE9\x00\x00\x00\x00", 5);
        Log("Patched GG check routines (SEA S1 2.16a)\r\n");
    }
}

void PatchGG_EU() {
    if (compare_virtual_memory(0x006160D1, 0xFF161AE8)) {
        Patch((LPVOID)0x006160D1, "\xE9\x00\x00\x00\x00", 5);
        Log("Patched GG check routines (EU S2 3.01a)\r\n");
    } else if (compare_virtual_memory(0x0075FB71, 0xFECECAE8)) {
        Patch((LPVOID)0x0075FB71, "\xE9\x00\x00\x00\x00", 5);
        Log("Patched GG check routines (EU S4 500)\r\n");
    }
}

void oneExec_PatchDynamicAndGG() {
    static int one = 0;
    PANGYAVER pangyaVersion;

    if (one == 1)
        return;

    one = 1;

    // GG and dynamic patch
    pangyaVersion = DetectPangyaVersion();

    // GG Patch
    switch (pangyaVersion) {
    case PANGYA_KR:
        PatchGG_KR();
        break;
    case PANGYA_JP:
        PatchGG_JP();
        break;
    case PANGYA_TH:
        PatchGG_TH();
        break;
    case PANGYA_ID:
        PatchHS_ID();
        break;
    case PANGYA_BR:
        PatchHS_BR();
        break;
    case PANGYA_TW:
        PatchGG_TW();
        break;
    case PANGYA_US:
        PatchGG_US();
        break;
    case PANGYA_EU:
        PatchGG_EU();
        break;
    case PANGYA_SEA:
        PatchGG_SEA();
        break;
    default:
        Warning("It looks like no patch exists for this version of PangYa™.\nThe game will likely "
                "exit a couple minutes after detecting GameGuard is not present.");
    }

    // Dynamic Patch
    PatchAddress();
}

BOOL STDCALL InitCommonControlsExHook(const INITCOMMONCONTROLSEX *picce) {
    BOOL ret = pInitCommonControlsEx(picce);

    oneExec_PatchDynamicAndGG();

    return ret;
}

VOID InitComCtl32Hook() {
    hComCtl32 = LoadLib("Comctl32");
    pInitCommonControlsEx = HookProc(hComCtl32, "InitCommonControlsEx", InitCommonControlsExHook);
}
