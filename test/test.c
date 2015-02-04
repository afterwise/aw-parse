
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "aw-parse.h"

char text[] =
	"{								\n"
	"	name: \"Suzie Cobol\"					\n"
	"	age: 73							\n"
	"	\"understated popularity\": -20.05			\n"
	"	caps-loco?:yes						\n"
	"	children: [						\n"
	"		{name: \"Angus\" age: 55},			\n"
	"		\"Lizzie\",\"Gunnar\"				\n"
	"		[ << !THIS WILL BE IGNORED! >> ]		\n"
	"	]							\n"
	"	break 							\n"
	"	<< !THIS WILL ALSO BE IGNORED! >>			\n"
	"}\n";

const char *tokens[] = {
        "PARSE_STOP",
        "PARSE_POP",

        "PARSE_BRACE",
        "PARSE_BRACKET",
        "PARSE_PAREN",

        "PARSE_COMMA",

        "PARSE_LET",
        "PARSE_SYM",
        "PARSE_STR",

        "PARSE_INT",
        "PARSE_FLOAT"
};

int main(int argc, char* argv[]) {
	char *str = text;
	union parse_value pv;
	enum parse_token pt;

	(void) argc;
	(void) argv;

	pt = parse_token(&pv, text, &str);
	assert(pt == PARSE_BRACE);

	while ((pt = parse_token(&pv, text, &str)) > PARSE_POP) {
		if (pt == PARSE_SYM && strcmp(pv.s, "break") == 0) {
			parse_skip_to_end(pt, text, &str);
			printf("Breaking out\n");
			break;
		}

		// printf("pt=%s <%s>\n", tokens[pt], pv.s);
		assert(pt == PARSE_LET);

		if (strcmp(pv.s, "name") == 0) {
			pt = parse_token(&pv, text, &str);
			assert(pt == PARSE_STR);
			printf("Name <%s>\n", pv.s);
		} else if (strcmp(pv.s, "age") == 0) {
			pt = parse_token(&pv, text, &str);
			assert(pt == PARSE_INT);
			printf("Age <%lld>\n", pv.i);
		} else if (strcmp(pv.s, "understated popularity") == 0) {
			pt = parse_token(&pv, text, &str);
			assert(pt == PARSE_FLOAT);
			printf("Understated Popularity <%f>\n", pv.f);
		} else if (strcmp(pv.s, "caps-loco?") == 0) {
			pt = parse_token(&pv, text, &str);
			assert(pt == PARSE_SYM);
			printf("Caps-loco? <%s>\n", pv.s);
		} else if (strcmp(pv.s, "children") == 0) {
			pt = parse_token(&pv, text, &str);
			assert(pt == PARSE_BRACKET);

			while ((pt = parse_token(&pv, text, &str)) > PARSE_POP) {
				if (pt == PARSE_BRACE) {
					while ((pt = parse_token(&pv, text, &str)) > PARSE_POP) {
						assert(pt == PARSE_LET);

						if (strcmp(pv.s, "name") == 0) {
							pt = parse_token(&pv, text, &str);
							assert(pt == PARSE_STR);
							printf("Child-Name <%s>\n", pv.s);
						} else if (strcmp(pv.s, "age") == 0) {
							pt = parse_token(&pv, text, &str);
							assert(pt == PARSE_INT);
							printf("Child-Age <%lld>\n", pv.i);
						} else {
							printf("Unexpected child attribute: %s\n", pv.s);
							return 1;
						}
					}
				} else if (pt == PARSE_STR) {
					printf("Child <%s>\n", pv.s);
				} else if (pt == PARSE_COMMA) {
					printf("Comma\n");
				} else {
					assert(pt == PARSE_BRACKET);
					parse_skip_token(pt, text, &str);
				}
			}
		} else {
			printf("Unexpected attribute: %s\n", pv.s);
			return 1;
		}
	}

	pt = parse_token(&pv, text, &str);
	assert(pt == PARSE_STOP);
	printf("OK\n");

	return 0;
}

