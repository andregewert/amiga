ccbin = vc
ccopts = -c99
includes = -I${NDK_INC}
libs = -lamiga -lauto
mathlib = -lmsoft
cc = $(ccbin) $(copts) $(includes) $(libs)

all: dirs window snakey

rebuild: clean dirs snakey


dirs:
	mkdir -p build

window: dirs window.c
		$(cc) window.c -o build/window

snakey: dirs snakey.c
		$(cc) snakey.c -o build/snakey

clean:
		rm -r build
