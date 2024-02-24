#include "../../bootstrap.h"
#include "../../common.h"
#include "../../hex.h"
#include "../../json.h"
#include "../../patch.h"
#include "../../regex.h"

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

void STDCALL DispatchTest(const DISPATCH_TESTCASE *_this, DWORD testnum) {
    ConsoleLog("ok %d - %s\r\n", testnum, _this->name);
}

void STDCALL DispatchThroughThiscallTest(const DISPATCH_TESTCASE *_this, DWORD testnum) {
    PFNDISPATCHTESTPROC pfnDispatchProc =
        (PFNDISPATCHTESTPROC)BuildStdcallToThiscallThunk(BuildThiscallToStdcallThunk(DispatchTest));
    pfnDispatchProc(_this, testnum);
}

const DISPATCH_TESTCASE dispatch_tests[] = {
    {"STDCALL -> THISCALL -> STDCALL", DispatchThroughThiscallTest},
};

#define COUNT_OF(x) ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))

extern void __cdecl start(void) {
    int i, result = 0, testnum = 0, totaltests = 0;

    totaltests += COUNT_OF(regex_tests);
    totaltests += COUNT_OF(addr_tests);
    totaltests += COUNT_OF(patch_tests);
    totaltests += COUNT_OF(dispatch_tests);

    BootstrapPEB();
    InitCommon();

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

    InitPatch();
    for (i = 0; i < COUNT_OF(dispatch_tests); ++i) {
        dispatch_tests[i].proc(&dispatch_tests[i], ++testnum);
    }

    Exit(result);
}
