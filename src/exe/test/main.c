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

#include "../../common.h"
#include "../../hex.h"
#include "../../json.h"
#include "../../patch.h"
#include "../../regex.h"

#include "../../../third_party/ijl/ijl.h"

typedef struct _REGEX_TESTCASE {
    LPCSTR pattern, replace, text, expected;
} REGEX_TESTCASE;

const REGEX_TESTCASE regex_tests[] = {
    {"([\\w]*)xx", "$0", "abcxx", "abc"},
    {"([a-c]+)([^a-c]+)([a-f]+)", "/$0/$1/$2/", "abcdeffabf", "/abc/deff/abf/"},
    {"abc([abc]*)abc", "$0", "abcabc", ""},
    {"abc([abc]*)abc", "$0", "abcabcabc", "abc"},
    {"(a)[a][b](b.)[c]", "$0$1", "aabbcc", "abc"},
    {"http://[^/]+/(.+)", "http://mycoolsite.net/$0",
     "http://user@pass:update.host.com:8080/update/updater.exe.zip",
     "http://mycoolsite.net/update/updater.exe.zip"},
    {"(.*)\\.js", "$0.ts", "index.js", "index.ts"},
};

typedef struct _PARSEADDR_TESTCASE {
    LPCSTR text;
    DWORD expected;
} PARSEADDR_TESTCASE;

const PARSEADDR_TESTCASE addr_tests[] = {
    {"0x0", 0x0u},
    {"0x1", 0x1u},
    {"0x10", 0x10u},
    {"0x0AF0", 0xAF0u},
    {"0x10000000", 0x10000000u},
    {"0x80000000", 0x80000000u},
    {"0xFFFFFFFF", 0xFFFFFFFFu},
    {"0x100000000", 0x0u}, // overflow
};

typedef struct _PARSEPATCH_TESTCASE {
    LPCSTR text, expected;
    DWORD expectedLen;
} PARSEPATCH_TESTCASE;

const PARSEPATCH_TESTCASE patch_tests[] = {
    {"\"lobby_gbin\"", "lobby_gbin", 10},
    // note: escaped twice: once for C, once for JSON decode
    {"\"lobby_gbin\\\\x00\"", "lobby_gbin\0", 11},
    {"\"lo\\\\x62\\\\x62\\\\x79_\\\\x77\\\\x65\\\\x73\\\\x74\\\\x2egbin\\\\x00\"",
     "lobby_west.gbin\0", 16},
    // Escaped *three* times: once for C, once for JSON decode, once for binary parse!
    {"\"test\\\\\\\\test\"", "test\\test", 9},
};

typedef struct _DISPATCH_TESTCASE DISPATCH_TESTCASE;

typedef void(STDCALL *PFNDISPATCHTESTPROC)(const DISPATCH_TESTCASE *_this, DWORD testnum);

typedef struct _DISPATCH_TESTCASE {
    LPCSTR name;
    PFNDISPATCHTESTPROC proc;
} DISPATCH_TESTCASE;

static void STDCALL DispatchTest(const DISPATCH_TESTCASE *_this, DWORD testnum) {
    ConsoleLog("ok %d - %s\r\n", testnum, _this->name);
}

static void STDCALL DispatchThroughThiscallTest(const DISPATCH_TESTCASE *_this, DWORD testnum) {
    PFNDISPATCHTESTPROC pfnDispatchProc =
        (PFNDISPATCHTESTPROC)BuildStdcallToThiscallThunk(BuildThiscallToStdcallThunk(DispatchTest));
    pfnDispatchProc(_this, testnum);
}

const DISPATCH_TESTCASE dispatch_tests[] = {
    {"STDCALL -> THISCALL -> STDCALL", DispatchThroughThiscallTest},
};

const LPCSTR ld_tests[] = {
    "5531d289e583ec14",       "5589e5538d45e4",         "5589e5575653",         "5589e583ec10",
    "55b911000000",           "6a1868b82f817c",         "6a2468685b1c77",       "8bff558bec33c0",
    "8bff558bec51",           "8bff558bec53",           "8bff558bec56",         "8bff558bec5d",
    "8bff558bec64a118000000", "8bff558bec64a130000000", "8bff558bec6801000040", "8bff558bec6a00",
    "8bff558bec83ec0c",       "8bff558bec83ec18",       "8bff558bec83ec3c",     "ff25241af476",
    "ff259017f476",
};

static BOOL ijl15_crash_test() {
    JPEG_CORE_PROPERTIES jcp;
    DWORD dwWholeImageSize, dwJPGSizeBytes;
    PVOID pJPGBytes;
    IJLERR jerr;

    // Load JPEG into memory
    pJPGBytes = ReadEntireFile("third_party/testdata/exception.jpg", &dwJPGSizeBytes);
    jerr = ijlInit(&jcp);
    if (jerr < 0) {
        return FALSE;
    }

    // Parse JPEG header
    jcp.JPGFile = 0;
    jcp.JPGBytes = pJPGBytes;
    jcp.JPGSizeBytes = dwJPGSizeBytes;
    ijlRead(&jcp, IJL_JBUFF_READPARAMS);
    if (jerr < 0) {
        return FALSE;
    }

    // Read JPEG image
    dwWholeImageSize = jcp.JPGWidth * jcp.JPGHeight * jcp.DIBChannels;
    PVOID buffer = AllocMem(dwWholeImageSize);
    jcp.DIBColor = IJL_G;
    jcp.DIBChannels = 1;
    jcp.DIBWidth = jcp.JPGWidth;
    jcp.DIBHeight = jcp.JPGHeight;
    jcp.DIBPadBytes = 0;
    jcp.DIBBytes = buffer;
    jerr = ijlRead(&jcp, IJL_JBUFF_READWHOLEIMAGE);
    FreeMem(buffer);

    // Return success if there was no error
    return jerr >= 0;
}

typedef BOOL (*UNIT_TEST)();

const UNIT_TEST unit_tests[] = {
    ijl15_crash_test,
};

#define COUNT_OF(x) ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))

extern void __cdecl start(void) {
    int i, result = 0, testnum = 0, totaltests = 0;

    totaltests += COUNT_OF(regex_tests);
    totaltests += COUNT_OF(addr_tests);
    totaltests += COUNT_OF(patch_tests);
    totaltests += COUNT_OF(dispatch_tests);
    totaltests += COUNT_OF(ld_tests);
    totaltests += COUNT_OF(unit_tests);

    ConsoleLog("1..%d\r\n", totaltests);

    for (i = 0; i < COUNT_OF(regex_tests); ++i) {
        REGEX_TESTCASE test = regex_tests[i];
        REGEX *re = ReParse(test.pattern);
        LPCSTR result = ReReplace(re, test.replace, test.text);
        ++testnum;
        if (!result) {
            ConsoleLog("not ok %d - ReReplace(%s, %s, %s)\r\n# expected %s to match %s\r\n",
                       testnum, test.pattern, test.replace, test.text, test.pattern, test.text);
            result++;
            continue;
        }
        if (strcmp(test.expected, result)) {
            ConsoleLog("not ok %d - ReReplace(%s, %s, %s)\r\n# expected %s to equal %s\r\n",
                       testnum, test.pattern, test.replace, test.text, result, test.expected);
            result++;
        } else {
            ConsoleLog("ok %d - ReReplace(%s, %s, %s)\r\n", testnum, test.pattern, test.replace,
                       test.text);
        }
        FreeMem((HLOCAL)result);
    }

    for (i = 0; i < COUNT_OF(addr_tests); ++i) {
        PARSEADDR_TESTCASE test = addr_tests[i];
        DWORD result = ParseAddress(test.text);
        ++testnum;
        if (result != test.expected) {
            ConsoleLog("not ok %d - ParseAddress(%s)\r\n# expected %08x to equal %08x\r\n", testnum,
                       test.text, result, test.expected);
        } else {
            ConsoleLog("ok %d - ParseAddress(%s)\r\n", testnum, test.text);
        }
    }

    for (i = 0; i < COUNT_OF(patch_tests); ++i) {
        PARSEPATCH_TESTCASE test = patch_tests[i];
        LPSTR rawInput = DupStr(test.text);
        LPSTR jsonPtr = rawInput;
        LPCSTR parsedInput = JsonReadString(&jsonPtr);
        DWORD resultLen;
        LPSTR result;
        ++testnum;
        ParsePatch(parsedInput, &result, &resultLen);
        FreeMem(rawInput);
        if (!result) {
            ConsoleLog("not ok %d - ParsePatch(%s)\r\n# unexpected null output\r\n", testnum,
                       test.text);
            continue;
        }
        if (test.expectedLen != resultLen) {
            ConsoleLog("not ok %d - ParsePatch(%s)\r\n# expected %s (length: %d) to equal %s "
                       "(length: %d)\r\n",
                       testnum, test.text, result, resultLen, test.expected, test.expectedLen);
        } else {
            if (memcmp(test.expected, result, resultLen)) {
                ConsoleLog("not ok %d - ParsePatch(%s)\r\n# expected %s to equal %s\r\n", testnum,
                           test.text, result, test.expected);
            } else {
                ConsoleLog("ok %d - ParsePatch(%s)\r\n", testnum, test.text);
            }
        }
        FreeMem(result);
    }

    for (i = 0; i < COUNT_OF(dispatch_tests); ++i) {
        dispatch_tests[i].proc(&dispatch_tests[i], ++testnum);
    }

    for (i = 0; i < COUNT_OF(ld_tests); ++i) {
        BYTE bTestBuffer[16];
        DWORD dwLengthExpected, dwLengthActual;
        ++testnum;
        memset(bTestBuffer, 0x90, sizeof(bTestBuffer));
        dwLengthExpected = FromHex(ld_tests[i], bTestBuffer);
        dwLengthActual = CountOpcodeBytes((PVOID)bTestBuffer, 6);
        if (dwLengthActual == dwLengthExpected) {
            ConsoleLog("ok %d - CountOpcodeBytes(%s)\r\n", testnum, ld_tests[i], dwLengthExpected);
        } else {
            ConsoleLog("not ok %d - CountOpcodeBytes(%s)\r\n# expected %d, got %d\r\n", testnum,
                       ld_tests[i], dwLengthExpected, dwLengthActual);
        }
    }

    for (i = 0; i < COUNT_OF(unit_tests); ++i) {
        ++testnum;
        if (unit_tests[i]()) {
            ConsoleLog("ok %d - unit test %d\r\n", testnum, i);
        } else {
            ConsoleLog("not ok %d - unit test %d\r\n", testnum, i);
        }
    }

    Exit(result);
}
