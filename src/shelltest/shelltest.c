#include <stdio.h>
#include <stdlib.h>
#include "../AppSupport/environment.h"

int main(int argc, char** argv) {
	printf("Color mode: %d\n", envGetColorMode());
	exit(EXIT_SUCCESS);
}
