#include <exec/types.h>
#include <hardware/custom.h>

#define PRA_FIR0_BIT    (1 << 6)
#define COLOR_WHITE     (0xfff)
#define COLOR_WBENCH_BG (0x05a)
#define TOP_POS         (0x40)
#define BOTTOM_POS      (0xf0)

extern struct Custom custom;
volatile UBYTE *ciaa_pra        = (volatile UBYTE *)0xbfe001;
volatile UBYTE *custom_vhposr   = (volatile UBYTE *)0xdff006;

int main(int argc, char **argv) {
    UBYTE pra_value = 0;
    UBYTE pos = 0xac;
    BYTE incr = 1;

    while ((*ciaa_pra & PRA_FIR0_BIT) != 0) {
        while (*custom_vhposr != 0xff);         // Wait for vblank
        while (*custom_vhposr != pos);          // Wait until position reached/
        custom.color[0] = COLOR_WHITE;
        while (*custom_vhposr == pos);          // Wait while on the target position
        custom.color[0] = COLOR_WBENCH_BG;
        if (pos >= BOTTOM_POS) incr = -incr;
        else if (pos <= TOP_POS) incr = -incr;
        pos += incr;
    }

    return 0;
}
