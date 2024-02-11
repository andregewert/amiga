#include "environment.h"

COLORMODE envGetColorMode() {
	STRPTR dummy[5];
	if (GetVar("shellcolors", dummy, 5, 0) > 0) {
		printf("shellcolor: %s\n", dummy);
		if (strcmp(dummy, "8") == 0) {
			return SHELL_EIGHT_COLORS;
		}
		if (strcmp(dummy, "4") == 0) {
			return SHELL_FOUR_COLORS;
		}
		if (strcmp(dummy, "full") == 0) {
			return SHELL_FULL_COLORS;
		}
	}
	return SHELL_NO_COLORS;
}
