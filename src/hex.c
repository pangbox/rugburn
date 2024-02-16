#include "hex.h"

DWORD ReadDword(LPCSTR _text) {

	if (_text == NULL) {
		FatalError("Hex - Read Dword hex in text, parameters invalid!");
	}

	if (_text[0] == '\0')
		return 0;

	LPSTR pos = (LPSTR)_text;
	DWORD dw = 0;
	int len = 0;

	if ((pos = strstr(pos, "0x")) == NULL) {
		FatalError("Hex - Read Dword hex in text, invalid dword hex!");
	}

	while (isxdigit(pos[len + 2]))
		len++;

	if (len > 0) {

		CHAR ctmp = pos[len + 2];
		pos[len + 2] = '\0';

		dw = strtol(pos + 2, NULL, 16);
	}

	return dw;
}

void TranslateHexInText(LPCSTR _text, LPSTR _result, int _size_result, int* _size_out) {

	if (_text == NULL || _result == NULL || _size_result == 0 || _size_out == NULL) {
		FatalError("Hex - Translate hex in text, parameters invalid!");
	}

	*_size_out = 0;

	if (_text[0] == '\0') {

		_result[*_size_out++] = '\0';

		return;
	}

	LPSTR pos = (LPSTR)_text;
	LPSTR rest = NULL;
	int len = 0, num = 0;

	do {

		rest = strstr(pos, "\\x");

		if (rest != NULL) {
			
			len = rest - pos;

			if (len > 0) {
				
				if ((*_size_out + len) > _size_result) {
					FatalError("Hex - Translate hex in text, overflow result buffer!");
				}

				memcpy(_result + *_size_out, pos, len);
				*_size_out += len;

				pos = rest;
			}

			len = 0;

			while (len < 2 && isxdigit(pos[len + 2]))
				len++;

			if (len > 0) {

				CHAR ctmp = pos[len + 2];
				pos[len + 2] = '\0';

				num = strtol(pos + 2, NULL, 16);

				if ((*_size_out + 1) > _size_result) {
					FatalError("Hex - Translate hex in text, overflow result buffer!");
				}

				pos += len + 2;
				*pos = ctmp;

				_result[*_size_out] = (CHAR)num;

				*_size_out += 1;

			}else {

				if ((*_size_out + 1) > _size_result) {
					FatalError("Hex - Translate hex in text, overflow result buffer!");
				}

				_result[*_size_out] = *pos++;

				*_size_out += 1;
			}
		
		}else {

			len = strlen(pos);

			if ((*_size_out + len) > _size_result) {
				FatalError("Hex - Translate hex in text, overflow result buffer!");
			}

			memcpy(_result + *_size_out, pos, len);
			*_size_out += len;

			pos += len;
		}

	} while(*pos != '\0');

	if ((*_size_out + 1) > _size_result) {
		FatalError("Hex - Translate hex in text, overflow result buffer!");
	}

	_result[*_size_out] = '\0';

	*_size_out += 1;
}
