#include "regex.h"

#define MAXRETOKENS 64
#define MAXRECHARCLASS 32
#define MAXRECAPTURE 10

typedef enum _RETOKENTYPE {
	RE_TOK_NONE,
	RE_TOK_LPAREN,
	RE_TOK_RPAREN,
	RE_TOK_PERIOD,
	RE_TOK_ASTERISK,
	RE_TOK_PLUS,
	RE_TOK_QUESTIONMARK,
	RE_TOK_CHAR,
	RE_TOK_CHARCLASS,
	RE_TOK_DIGIT,
	RE_TOK_NONDIGIT,
	RE_TOK_ALPHA,
	RE_TOK_NONALPHA,
	RE_TOK_WHITESPACE,
	RE_TOK_NONWHITESPACE,
} RETOKENTYPE;

typedef struct _RETOKEN {
	RETOKENTYPE type;
	union {
		BYTE ch;
		BYTE *charClass;
	};
} RETOKEN;

typedef struct _RECAPGROUP {
	LPCSTR str;
	int len;
} RECAPGROUP;

struct _REGEX {
	RETOKEN tok[MAXRETOKENS];
	BYTE charClass[MAXRECHARCLASS];
	RECAPGROUP cap[MAXRECAPTURE];
	int numCap;
};

static BOOL ReMatchPattern(REGEX *regex, RETOKEN *pattern, LPCSTR text);
static BOOL ReMatchChClass(CHAR ch, LPCSTR str);
static BOOL ReMatchAsterisk(REGEX *regex, RETOKEN p, RETOKEN *pattern, LPCSTR text);
static BOOL ReMatchPlus(REGEX *regex, RETOKEN p, RETOKEN *pattern, LPCSTR text);
static BOOL ReMatchOne(RETOKEN p, CHAR ch);
static BOOL ReMatchDigit(CHAR ch);
static BOOL ReMatchAlpha(CHAR ch);
static BOOL ReMatchWhitespace(CHAR ch);
static BOOL ReMatchMetaChar(CHAR ch, LPCSTR str);
static BOOL ReMatchRange(CHAR ch, LPCSTR str);
static BOOL ReIsMetaCh(CHAR ch);

int ReGetNumCaptures(REGEX *pattern) {
	return pattern->numCap;
}

int ReGetCaptureLen(REGEX *pattern, int i) {
	return pattern->cap[i].len;
}

void ReGetCaptureData(REGEX *pattern, int i, LPSTR buffer) {
	memcpy(buffer, pattern->cap[i].str, pattern->cap[i].len);
}

BOOL ReMatch(REGEX *pattern, LPCSTR text) {
	pattern->numCap = 0;
	memset(pattern->cap, 0, sizeof(RECAPGROUP) * MAXRECAPTURE);
	return ReMatchPattern(pattern, &pattern->tok[0], text);
}

LPSTR ReReplace(REGEX *find, LPCSTR replace, LPCSTR text) {
	LPCSTR ptr;
	LPSTR result, ptrout;
	int outLen = 0;

	if (!ReMatch(find, text)) {
		return NULL;
	}

	// Preflight
	ptr = replace;
	outLen = 0;
	while (*ptr != 0) {
		switch (*ptr++) {
		case '$':
			if (*ptr >= '0' && *ptr <= '9') {
				outLen += find->cap[*ptr-'0'].len;
			} else if (*ptr == '$') {
				outLen++;
			}
			ptr++;
			break;
		default:
			outLen++;
		}
	}
	outLen++;

	// Substitution
	result = LocalAlloc(0, outLen);
	ptr = replace;
	ptrout = result;
	while (*ptr != 0) {
		switch (*ptr) {
		case '$':
			ptr++;
			if (*ptr >= '0' && *ptr <= '9') {
				memcpy(ptrout, find->cap[*ptr-'0'].str, find->cap[*ptr-'0'].len);
				ptrout += find->cap[*ptr-'0'].len;
			} else if (*ptr == '$') {
				*ptrout++ = '$';
			}
			ptr++;
			break;
		default:
			*ptrout++ = *ptr++;
		}
	}
	*ptrout = 0;
	return result;
}

REGEX *ReParse(LPCSTR pattern) {
	REGEX *compiled = (REGEX *)LocalAlloc(0, sizeof(REGEX));
	int charClassIndex = 1,  i = 0, j = 0;
	CHAR ch;

	ZeroMemory(compiled, sizeof(REGEX));

	while (pattern[i] != 0) {
		if (j+1 >= MAXRETOKENS) {
			FatalError("Parsing regular expression: reached max number of tokens.");
		}
		ch = pattern[i];
		switch (ch) {
			case '(': { compiled->tok[j].type = RE_TOK_LPAREN; } break;
			case ')': { compiled->tok[j].type = RE_TOK_RPAREN; } break;
			case '.': { compiled->tok[j].type = RE_TOK_PERIOD; } break;
			case '*': { compiled->tok[j].type = RE_TOK_ASTERISK; } break;
			case '+': { compiled->tok[j].type = RE_TOK_PLUS; } break;
			case '?': { compiled->tok[j].type = RE_TOK_QUESTIONMARK; } break;
			case '\\': {
				if (pattern[i+1] != 0) {
					i++;
					switch (pattern[i]) {
						case 'd': { compiled->tok[j].type = RE_TOK_DIGIT; } break;
						case 'D': { compiled->tok[j].type = RE_TOK_NONDIGIT; } break;
						case 'w': { compiled->tok[j].type = RE_TOK_ALPHA; } break;
						case 'W': { compiled->tok[j].type = RE_TOK_NONALPHA; } break;
						case 's': { compiled->tok[j].type = RE_TOK_WHITESPACE; } break;
						case 'S': { compiled->tok[j].type = RE_TOK_NONWHITESPACE; } break;
						default: {
							compiled->tok[j].type = RE_TOK_CHAR;
							compiled->tok[j].ch = pattern[i];
						} break;
					}
				}
			} break;
			case '[': {
				int charClassBegin = charClassIndex;
				compiled->tok[j].type = RE_TOK_CHARCLASS;
				while ((pattern[++i] != ']') && (pattern[i]   != 0)) {
					if (pattern[i] == '\\') {
						if (charClassIndex >= MAXRECHARCLASS - 1) {
			        FatalError("Parsing regular expression: reached max character class storage.");
						}
						compiled->charClass[charClassIndex++] = pattern[i++];
					} else if (charClassIndex >= MAXRECHARCLASS) {
						FatalError("Parsing regular expression: reached max character class storage.");
					}
					compiled->charClass[charClassIndex++] = pattern[i];
				}
				if (charClassIndex >= MAXRECHARCLASS) {
					FatalError("Parsing regular expression: reached max character class storage.");
				}
				compiled->charClass[charClassIndex++] = 0;
				compiled->tok[j].charClass = &compiled->charClass[charClassBegin];
			}
			break;
			default: {
				compiled->tok[j].type = RE_TOK_CHAR;
				compiled->tok[j].ch = ch;
			} break;
		}
		i++;
		j++;
	}
	compiled->tok[j].type = RE_TOK_NONE;
	return compiled;
}

static BOOL ReMatchDigit(CHAR ch) {
	return ch >= '0' && ch <= '9';
}
static BOOL ReMatchAlpha(CHAR ch) {
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}
static BOOL ReMatchWhitespace(CHAR ch) {
	return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\f' || ch == '\v';
}
static BOOL ReMatchAlphaNum(CHAR ch) {
	return ch == '_' || ReMatchAlpha(ch) || ReMatchDigit(ch);
}
static BOOL ReMatchRange(CHAR ch, LPCSTR str) {
	return ch != '-' && str[0] != 0 && str[0] != '-' && str[1] == '-' && str[1] != 0 && str[2] != 0 && (ch >= str[0] && ch <= str[2]);
}
static BOOL ReIsMetaCh(CHAR ch) {
	return ch == 's' || ch == 'S' || ch == 'd' || ch == 'D' || ch == 'w' || ch == 'W';
}

static BOOL ReMatchMetaChar(CHAR ch, LPCSTR str) {
	switch (str[0]) {
		case 'd': return ReMatchDigit(ch);
		case 'D': return !ReMatchDigit(ch);
		case 'w': return ReMatchAlphaNum(ch);
		case 'W': return !ReMatchAlphaNum(ch);
		case 's': return ReMatchWhitespace(ch);
		case 'S': return !ReMatchWhitespace(ch);
		default: return ch == str[0];
	}
}

static BOOL ReMatchChClass(CHAR ch, LPCSTR str) {
	do {
		if (ReMatchRange(ch, str)) {
			return TRUE;
		} else if (str[0] == '\\') {
			str++;
			if (ReMatchMetaChar(ch, str)) {
				return TRUE;
			} else if (ch == str[0] && !ReIsMetaCh(ch)) {
				return TRUE;
			}
		} else if (ch == str[0]) {
			if (ch == '-') {
				return str[-1] == 0 || str[1] == 0;
			} else {
				return TRUE;
			}
		}
	} while (*str++ != 0);
	return FALSE;
}

static BOOL ReMatchOne(RETOKEN p, CHAR ch) {
	switch (p.type) {
		case RE_TOK_PERIOD: return TRUE;
		case RE_TOK_CHARCLASS: return ReMatchChClass(ch, (LPCSTR)p.charClass);
		case RE_TOK_DIGIT: return ReMatchDigit(ch);
		case RE_TOK_NONDIGIT: return !ReMatchDigit(ch);
		case RE_TOK_ALPHA: return ReMatchAlphaNum(ch);
		case RE_TOK_NONALPHA: return !ReMatchAlphaNum(ch);
		case RE_TOK_WHITESPACE: return ReMatchWhitespace(ch);
		case RE_TOK_NONWHITESPACE: return !ReMatchWhitespace(ch);
		default: return (p.ch == ch);
	}
}

static BOOL ReMatchAsterisk(REGEX *regex, RETOKEN p, RETOKEN *pattern, LPCSTR text) {
	LPCSTR start = text;
	while (text[0] != 0 && ReMatchOne(p, *text)) {
		text++;
	}
	while (text >= start) {
		if (ReMatchPattern(regex, pattern, text--)) {
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL ReMatchPlus(REGEX *regex, RETOKEN p, RETOKEN *pattern, LPCSTR text) {
	LPCSTR start = text;
	while (text[0] != 0 && ReMatchOne(p, *text)) {
		text++;
	}
	while (text > start) {
		if (ReMatchPattern(regex, pattern, text--)) {
			return 1;
		}
	}  
	return 0;
}

static BOOL ReMatchQuestionMark(REGEX *regex, RETOKEN p, RETOKEN *pattern, LPCSTR text) {
	if (p.type == RE_TOK_NONE) {
		return TRUE;
	}
	if (ReMatchPattern(regex, pattern, text)) {
		return TRUE;
	}
	if (*text && ReMatchOne(p, *text++)) {
		if (ReMatchPattern(regex, pattern, text)) {
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL ReMatchPattern(REGEX *regex, RETOKEN *pattern, LPCSTR text) {
	do {
		if (pattern[0].type == RE_TOK_LPAREN) {
			if (regex->numCap == MAXRECAPTURE) {
        FatalError("Matching regular expression: reached max capture groups.");
			}
			regex->cap[regex->numCap++].str = text;
			pattern++;
		}
		if (regex->numCap > 0 && regex->cap[regex->numCap - 1].str && pattern[0].type == RE_TOK_RPAREN) {
			regex->cap[regex->numCap - 1].len = text - regex->cap[regex->numCap - 1].str;
			pattern++;
		}
		if (pattern[1].type == RE_TOK_QUESTIONMARK) {
			return ReMatchQuestionMark(regex, pattern[0], &pattern[2], text);
		} else if (pattern[1].type == RE_TOK_ASTERISK) {
			return ReMatchAsterisk(regex, pattern[0], &pattern[2], text);
		} else if (pattern[1].type == RE_TOK_PLUS) {
			return ReMatchPlus(regex, pattern[0], &pattern[2], text);
		} else if (pattern[1].type == RE_TOK_NONE) {
			return text[0] == 0;
		}
	} while (text[0] != 0 && ReMatchOne(*pattern++, *text++));
	return FALSE;
}
