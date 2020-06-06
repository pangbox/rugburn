#include "common.h"
#include "regex.h"

typedef struct _TESTCASE {
	LPCSTR pattern, replace, text, result;
} TESTCASE;

const TESTCASE tests[] = {
	{"([\\w]*)xx", "$0", "abcxx", "abc"},
	{"([a-c]+)([^a-c]+)([a-f]+)", "/$0/$1/$2/", "abcdeffabf", "/abc/deff/abf/"},
	{"abc([abc]*)abc", "$0", "abcabc", ""},
	{"abc([abc]*)abc", "$0", "abcabcabc", "abc"},
	{"(a)[a][b](b.)[c]", "$0$1", "aabbcc", "abc"},
	{"http://[^/]+/(.+)", "http://mycoolsite.net/$0", "http://user@pass:update.host.com:8080/update/updater.exe.zip", "http://mycoolsite.net/update/updater.exe.zip"},
	{"(.*)\\.js", "$0.ts", "index.js", "index.ts"},
};

INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
	int i, result = 0;
	for (i = 0; i < sizeof(tests)/sizeof(TESTCASE); ++i) {
		REGEX *re = ReParse(tests[i].pattern);
		LPCSTR result = ReReplace(re, tests[i].replace, tests[i].text);
		if (!result) {
			Warning("Expected %s to match %s, but it didn't.", tests[i].pattern, tests[i].text);
			result++;
			continue;
		}
		if (strcmp(tests[i].result, result)) {
			Warning("Expected %s, got %s", tests[i].result, result);
			result++;
		}
		LocalFree((HLOCAL)result);
	}
	if (result > 0) {
		Log("Failed %d tests.", result);
	}
	return result;
}
