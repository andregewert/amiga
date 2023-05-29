ccbin = vc
ccopts = -c99
includes = -I${NDK_INC}
libs = -lamiga -lauto
mathlib = -lmsoft
cc = $(ccbin) $(ccopts) $(includes) $(libs)

all: dirs window snakey

rebuild: clean dirs snakey


dirs:
	mkdir -p build

window: dirs window.c
		$(cc) window.c -o build/window

snakey: dirs snakey.c
		$(cc) snakey.c -o build/snakey

lang: dirs lang.c
		$(cc) lang.c -o build/lang

clean:
		rm -r build
