
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "aw-parse.h"

char text[] =
	"{								\n"
	"	name: \"Suzie Cobol\"					\n"
	"	age: 73							\n"
	"	popularity: -20.05					\n"
	"	caps-loco?: yes						\n"
	"	children: [						\n"
	"		{name: \"Angus\" age: 55}			\n"
	"		\"Lizzie\"					\n"
	"		[ << !THIS WILL BE IGNORED! >> ]		\n"
	"	]							\n"
	"	break 							\n"
	"	<< !THIS WILL ALSO BE IGNORED! >>			\n"
	"}\n";

int main(int argc, char* argv[]) {
	char *str = text;
	union parse_value pv;
	enum parse_token pt;

	pt = parse_token(&pv, str, &str);
	assert(pt == PARSE_BRACE);

	while ((pt = parse_token(&pv, str, &str)) > PARSE_POP) {
		if (pt == PARSE_SYM && strcmp(pv.s, "break") == 0) {
			parse_skip_to_end(pt, str, &str);
			printf("Breaking out\n");
			break;
		}

		assert(pt == PARSE_LET);

		if (strcmp(pv.s, "name") == 0) {
			pt = parse_token(&pv, str, &str);
			assert(pt == PARSE_STR);
			printf("Name <%s>\n", pv.s);
		} else if (strcmp(pv.s, "age") == 0) {
			pt = parse_token(&pv, str, &str);
			assert(pt == PARSE_INT);
			printf("Age <%d>\n", pv.i);
		} else if (strcmp(pv.s, "popularity") == 0) {
			pt = parse_token(&pv, str, &str);
			assert(pt == PARSE_FLOAT);
			printf("Popularity <%f>\n", pv.f);
		} else if (strcmp(pv.s, "caps-loco?") == 0) {
			pt = parse_token(&pv, str, &str);
			assert(pt == PARSE_SYM);
			printf("Caps-loco? <%s>\n", pv.s);
		} else if (strcmp(pv.s, "children") == 0) {
			pt = parse_token(&pv, str, &str);
			assert(pt == PARSE_BRACKET);

			while ((pt = parse_token(&pv, str, &str)) > PARSE_POP) {
				if (pt == PARSE_BRACE) {
					while ((pt = parse_token(&pv, str, &str)) > PARSE_POP) {
						assert(pt == PARSE_LET);

						if (strcmp(pv.s, "name") == 0) {
							pt = parse_token(&pv, str, &str);
							assert(pt == PARSE_STR);
							printf("Child-Name <%s>\n", pv.s);
						} else if (strcmp(pv.s, "age") == 0) {
							pt = parse_token(&pv, str, &str);
							assert(pt == PARSE_INT);
							printf("Child-Age <%d>\n", pv.i);
						} else {
							printf("Unexpected child attribute: %s\n", pv.s);
							return 1;
						}
					}
				} else if (pt == PARSE_STR) {
					printf("Child <%s>\n", pv.s);
				} else {
					assert(pt == PARSE_BRACKET);
					parse_skip_token(pt, str, &str);
				}
			}
		} else {
			printf("Unexpected attribute: %s\n", pv.s);
			return 1;
		}
	}

	pt = parse_token(&pv, str, &str);
	assert(pt == PARSE_STOP);
	printf("OK\n");

	return 0;
}

