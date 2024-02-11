#include <stdio.h>
#include <stdint.h>
#include <exec/types.h>
#define PRA_FIR0_BIT (1 << 6)

volatile UBYTE *ciaa_pra = (volatile UBYTE *)0xbfe001;

void waitmouse(void) {
    while ((*ciaa_pra & PRA_FIR0_BIT) != 0);
}

int main(int argc, char **argv) {
    printf("Waiting for mouse button\n");
    waitmouse();
    printf("Exiting ...\n");
    return 0;
}
