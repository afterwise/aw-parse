
/*
   Copyright (c) 2014 Malte Hildingsson, malte (at) afterwi.se

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
 */

#include <string.h>
#include "aw-parse.h"

enum parse_token parse_symbol(union parse_value *pv, char *str, char **end) {
	char *it = str;

	for (;;)
		switch (*it) {
		case ':':
			*it = 0;

			if (end != NULL)
				*end = it + 2;

			pv->s = str;
			return PARSE_LET;

		case ' ':
		case '\t':
		case '\f':
		case '\r':
		case '\n':
			*it = 0;

			if (end != NULL)
				*end = it + 1;

			pv->s = str;
			return PARSE_SYM;

		case '\0':
			if (end != NULL)
				*end = it;

			pv->s = str;
			return PARSE_SYM;

		default:
			it++;
		}
}

enum parse_token parse_string(union parse_value *pv, char *str, char **end) {
	char *it = str, *jt;

	if (*it++ != '"')
		return PARSE_STOP;

	for (jt = it;;)
		switch (*it) {
		case '"':
			*jt = 0;

			if (end != NULL)
				*end = it + 1;

			pv->s = str + 1;
			return PARSE_STR;

		case '\\':
			switch (it[1]) {
			case '"':
				it += 2;
				*jt++ = '"';
				break;

			case '\\':
				it += 2;
				*jt++ = '\\';
				break;

			case 'b':
				it += 2;
				*jt++ = '\b';
				break;

			case 'f':
				it += 2;
				*jt++ = '\f';
				break;

			case 'r':
				it += 2;
				*jt++ = '\r';
				break;

			case 'n':
				it += 2;
				*jt++ = '\n';
				break;

			case 't':
				it += 2;
				*jt++ = '\t';
				break;

			case 'u':
				/* XXX: not implemented */

			default:
				return PARSE_STOP;
			}
			break;

		case '\0':
			return PARSE_STOP;

		default:
			*jt++ = *it++;
		}
}

enum parse_token parse_number(union parse_value *pv, char *str, char **end) {
	char *it = str;
	char *jt = it;
	enum parse_token pt = PARSE_INT;
	int i, sgn = 1, iv = 0;
	float fv = 0.f;
	long x;

	for (;;)
		switch (*it) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			iv = (iv << 1) + (iv << 3) + (*it++ ^ '0');
			break;

		case '.':
			jt = ++it;
			fv = iv;
			iv = 0;
			pt = PARSE_FLOAT;
			break;

		case '-':
			sgn = -1;

			/* fall through */

		case '+':
			if (it == str) {
				it++;
				break;
			}

			/* fall through */

		default:
			if (pt == PARSE_FLOAT) {
				if (iv > 0) {
					x = 1;

					for (i = 0; i < it - jt; ++i)
						x *= 10;

					fv += (float) iv / (float) x;
				}

				pv->f = sgn >= 0 ? fv : -fv;
			} else
				pv->i = sgn >= 0 ? iv : -iv;

			if (end != NULL)
				*end = it;

			return pt;
		}
}

enum parse_token parse_token(union parse_value *pv, char *str, char **end) {
	char *it = str;

	for (;;)
		switch (*it) {
		case '{':
			*end = it + 1;
			return PARSE_BRACE;

		case '[':
			*end = it + 1;
			return PARSE_BRACKET;

		case '(':
			*end = it + 1;
			return PARSE_PAREN;

		case '}':
		case ']':
		case ')':
			*end = it + 1;
			return PARSE_POP;

		case '"':
			return parse_string(pv, it, end);

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			return parse_number(pv, it, end);

		case ' ':
		case '\t':
		case '\f':
		case '\r':
		case '\n':
			it++;
			break;

		case '\0':
			return PARSE_STOP;

		case '-':
		case '+':
			if ((unsigned) (it[1] - '0') <= 9)
				return parse_number(pv, it, end);

			/* fall through */

		default:
			return parse_symbol(pv, it, end);
		}
}

void parse_skip_token(enum parse_token pt, char *str, char **end) {
	union parse_value pv;
	int depth;

	if (pt == PARSE_BRACE || pt == PARSE_BRACKET)
		for (depth = 0;;) {
			switch (parse_token(&pv, str, end)) {
			case PARSE_BRACE:
			case PARSE_BRACKET:
			case PARSE_PAREN:
				depth++;
				break;

			case PARSE_LET:
			case PARSE_SYM:
			case PARSE_STR:
			case PARSE_INT:
			case PARSE_FLOAT:
				break;

			case PARSE_POP:
				if (depth-- > 0)
					break;

				/* fall through */

			case PARSE_STOP:
				return;
			}

			str = *end;
		}
}

unsigned parse_skip_to_end(enum parse_token pt, char *str, char **end) {
	union parse_value pv;
	unsigned count = 0;

	if (pt > PARSE_POP)
		while ((pt = parse_token(&pv, str, &str)) > PARSE_POP) {
			parse_skip_token(pt, str, &str);
			++count;
		}

	if (end != NULL)
		*end = str;

	return count;
}
