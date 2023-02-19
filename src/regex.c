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
	RE_TOK_INVCHARCLASS,
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

static BOOL ReMatchPattern(REGEX *pattern, RETOKEN *tokenptr, LPCSTR text);

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

LPSTR ReReplace(REGEX *pattern, LPCSTR replace, LPCSTR text) {
	LPCSTR ptr;
	LPSTR result, ptrout;
	int outLen = 0;

	if (!ReMatch(pattern, text)) {
		return NULL;
	}

	// Preflight
	ptr = replace;
	outLen = 0;
	while (*ptr != 0) {
		switch (*ptr++) {
		case '$':
			if (*ptr >= '0' && *ptr <= '9') {
				outLen += pattern->cap[*ptr-'0'].len;
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
	result = AllocMem(outLen);
	ptr = replace;
	ptrout = result;
	while (*ptr != 0) {
		switch (*ptr) {
		case '$':
			ptr++;
			if (*ptr >= '0' && *ptr <= '9') {
				memcpy(ptrout, pattern->cap[*ptr-'0'].str, pattern->cap[*ptr-'0'].len);
				ptrout += pattern->cap[*ptr-'0'].len;
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
	REGEX *compiled = (REGEX *)AllocMem(sizeof(REGEX));
	int charClassIndex = 1, charClassBegin = 0, i = 0, j = 0;
	CHAR ch;

	ZeroMemory(compiled, sizeof(REGEX));

	while (pattern[i] != 0) {
		if (j+1 >= MAXRETOKENS) {
			FatalError("Parsing regular expression: reached max number of tokens.");
		}
		ch = pattern[i];
		switch (ch) {
		case '(':
			compiled->tok[j].type = RE_TOK_LPAREN;
			break;
		case ')':
			compiled->tok[j].type = RE_TOK_RPAREN;
			break;
		case '.':
			compiled->tok[j].type = RE_TOK_PERIOD;
			break;
		case '*':
			compiled->tok[j].type = RE_TOK_ASTERISK;
			break;
		case '+':
			compiled->tok[j].type = RE_TOK_PLUS;
			break;
		case '?':
			compiled->tok[j].type = RE_TOK_QUESTIONMARK;
			break;
		case '\\':
			if (pattern[i+1] != 0) {
				i++;
				switch (pattern[i]) {
				case 'd':
					compiled->tok[j].type = RE_TOK_DIGIT;
					break;
				case 'D':
					compiled->tok[j].type = RE_TOK_NONDIGIT;
					break;
				case 'w':
					compiled->tok[j].type = RE_TOK_ALPHA;
					break;
				case 'W':
					compiled->tok[j].type = RE_TOK_NONALPHA;
					break;
				case 's':
					compiled->tok[j].type = RE_TOK_WHITESPACE;
					break;
				case 'S':
					compiled->tok[j].type = RE_TOK_NONWHITESPACE;
					break;
				default:
					compiled->tok[j].type = RE_TOK_CHAR;
					compiled->tok[j].ch = pattern[i];
					break;
				}
			}
			break;
		case '[':
			charClassBegin = charClassIndex;
			compiled->tok[j].type = RE_TOK_CHARCLASS;

			if (pattern[++i] == '^') {
				compiled->tok[j].type = RE_TOK_INVCHARCLASS;
				i++;
			}

			while (pattern[i] != ']' && pattern[i] != 0) {
				if (pattern[i] == '\\') {
					if (charClassIndex >= MAXRECHARCLASS - 1) {
						FatalError("Parsing regular expression: reached max character class storage.");
					}
					compiled->charClass[charClassIndex++] = pattern[i++];
				} else if (charClassIndex >= MAXRECHARCLASS) {
					FatalError("Parsing regular expression: reached max character class storage.");
				}
				compiled->charClass[charClassIndex++] = pattern[i];
				i++;
			}
			if (charClassIndex >= MAXRECHARCLASS) {
				FatalError("Parsing regular expression: reached max character class storage.");
			}
			compiled->charClass[charClassIndex++] = 0;
			compiled->tok[j].charClass = &compiled->charClass[charClassBegin];
			break;

		default:
			compiled->tok[j].type = RE_TOK_CHAR;
			compiled->tok[j].ch = ch;
			break;
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

static BOOL ReMatchAlphaNum(CHAR ch) {
	return ch == '_' || ReMatchAlpha(ch) || ReMatchDigit(ch);
}

static BOOL ReMatchWhitespace(CHAR ch) {
	return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\f' || ch == '\v';
}

static BOOL ReMatchRange(CHAR ch, LPCSTR str) {
	return ch != '-' && str[0] != 0 && str[0] != '-' && str[1] == '-' && str[1] != 0 && str[2] != 0 && (ch >= str[0] && ch <= str[2]);
}

static BOOL ReIsMetaCh(CHAR ch) {
	return ch == 's' || ch == 'S' || ch == 'd' || ch == 'D' || ch == 'w' || ch == 'W';
}

static BOOL ReMatchMetaChar(CHAR ch, LPCSTR str) {
	switch (str[0]) {
	case 'd':
		return ReMatchDigit(ch);
	case 'D':
		return !ReMatchDigit(ch);
	case 'w':
		return ReMatchAlphaNum(ch);
	case 'W':
		return !ReMatchAlphaNum(ch);
	case 's':
		return ReMatchWhitespace(ch);
	case 'S':
		return !ReMatchWhitespace(ch);
	default:
		return ch == str[0];
	}
}

static BOOL ReMatchCharClass(CHAR ch, LPCSTR str) {
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
	case RE_TOK_PERIOD:
		return TRUE;
	case RE_TOK_CHARCLASS:
		return ReMatchCharClass(ch, (LPCSTR)p.charClass);
	case RE_TOK_INVCHARCLASS:
		return !ReMatchCharClass(ch, (LPCSTR)p.charClass);
	case RE_TOK_DIGIT:
		return ReMatchDigit(ch);
	case RE_TOK_NONDIGIT:
		return !ReMatchDigit(ch);
	case RE_TOK_ALPHA:
		return ReMatchAlphaNum(ch);
	case RE_TOK_NONALPHA:
		return !ReMatchAlphaNum(ch);
	case RE_TOK_WHITESPACE:
		return ReMatchWhitespace(ch);
	case RE_TOK_NONWHITESPACE:
		return !ReMatchWhitespace(ch);
	default:
		return p.ch == ch;
	}
}

static BOOL ReMatchZeroOrMore(REGEX *pattern, RETOKEN p, RETOKEN *tokenptr, LPCSTR text) {
	LPCSTR start = text;
	while (text[0] != 0 && ReMatchOne(p, *text)) {
		text++;
	}
	while (text >= start) {
		if (ReMatchPattern(pattern, tokenptr, text--)) {
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL ReMatchOneOrMore(REGEX *pattern, RETOKEN p, RETOKEN *tokenptr, LPCSTR text) {
	LPCSTR start = text;
	while (text[0] != 0 && ReMatchOne(p, *text)) {
		text++;
	}
	while (text > start) {
		if (ReMatchPattern(pattern, tokenptr, text--)) {
			return TRUE;
		}
	}  
	return FALSE;
}

static BOOL ReMatchOptional(REGEX *pattern, RETOKEN p, RETOKEN *tokenptr, LPCSTR text) {
	if (p.type == RE_TOK_NONE) {
		return TRUE;
	}
	if (ReMatchPattern(pattern, tokenptr, text)) {
		return TRUE;
	}
	if (*text && ReMatchOne(p, *text++)) {
		if (ReMatchPattern(pattern, tokenptr, text)) {
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL ReMatchPattern(REGEX *pattern, RETOKEN *tokenptr, LPCSTR text) {
	do {
		// End capture group.
		if (pattern->numCap > 0 && pattern->cap[pattern->numCap - 1].str && tokenptr[0].type == RE_TOK_RPAREN) {
			pattern->cap[pattern->numCap - 1].len = text - pattern->cap[pattern->numCap - 1].str;
			tokenptr++;
		}
	
		// Start capture group.
		if (tokenptr[0].type == RE_TOK_LPAREN) {
			if (pattern->numCap == MAXRECAPTURE) {
				FatalError("Matching regular expression: reached max capture groups.");
			}
			pattern->cap[pattern->numCap++].str = text;
			tokenptr++;
		}

		if (tokenptr[1].type == RE_TOK_QUESTIONMARK) {
			return ReMatchOptional(pattern, tokenptr[0], &tokenptr[2], text);
		} else if (tokenptr[1].type == RE_TOK_ASTERISK) {
			return ReMatchZeroOrMore(pattern, tokenptr[0], &tokenptr[2], text);
		} else if (tokenptr[1].type == RE_TOK_PLUS) {
			return ReMatchOneOrMore(pattern, tokenptr[0], &tokenptr[2], text);
		} else if (tokenptr[0].type == RE_TOK_NONE) {
			return text[0] == 0;
		}
	} while (text[0] != 0 && ReMatchOne(*tokenptr++, *text++));
	return FALSE;
}
