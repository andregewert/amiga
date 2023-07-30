ccbin = vc
ccopts = -c99
includes = -I${NDK_INC}
libs = -lamiga -lauto

mathlib = -lmsoft
cc = $(ccbin) $(ccopts) $(includes) $(libs)

all: dirs engine window snakey lang shellcolors startup

rebuild: clean all


dirs:
	mkdir -p build

engine: dirs engine7.c
		$(cc) $(mathlib) engine7.c -o build/engine7

window: dirs window.c
		$(cc) window.c -o build/window

snakey: dirs snakey.c
		$(cc) snakey.c -o build/snakey

lang: dirs lang.c
		$(cc) lang.c -o build/lang

shellcolors: dirs shellcolors.c
		$(cc) shellcolors.c -o build/shellcolors

startup: dirs startup.c
		$(cc) startup.c -o build/startup

clean:
		rm -r build
