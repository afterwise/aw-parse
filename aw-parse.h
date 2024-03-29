
/*
   Copyright (c) 2014-2021 Malte Hildingsson, malte (at) afterwi.se

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

#ifndef AW_PARSE_H
#define AW_PARSE_H

#if defined(_parse_dllexport)
# if _MSC_VER
#  define _parse_api extern __declspec(dllexport)
# elif __GNUC__
#  define _parse_api __attribute__((visibility("default"))) extern
# endif
#elif defined(_parse_dllimport)
# if _MSC_VER
#  define _parse_api extern __declspec(dllimport)
# endif
#endif
#ifndef _parse_api
# define _parse_api extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum parse_token {
	PARSE_STOP,
	PARSE_POP,

	PARSE_BRACE,
	PARSE_BRACKET,
	PARSE_PAREN,

	PARSE_COMMA,

	PARSE_LET,
	PARSE_SYM,
	PARSE_STR,

	PARSE_INT,
	PARSE_FLOAT
};

union parse_value {
	char *s;
	long long i;
	double f;
};

_parse_api enum parse_token parse_symbol(union parse_value *pv, char *str, char **end);
_parse_api enum parse_token parse_string(union parse_value *pv, char *str, char **end);
_parse_api enum parse_token parse_number(union parse_value *pv, char **end);
_parse_api enum parse_token parse_token(union parse_value *pv, char *str, char **end);

_parse_api void parse_skip_token(enum parse_token pt, char *str, char **end);
_parse_api unsigned parse_skip_to_end(enum parse_token pt, char *str, char **end);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* AW_PARSE_H */

