ccbin = vc
ccopts = -c99
includes = -I${NDK_INC}
libs = -lamiga -lauto

mathlib = -lmsoft
cc = $(ccbin) $(ccopts) $(includes) $(libs)

prepare:
	mkdir -p build

clean:
	rm -r build

main: prepare main.c functions.c functions.h
	$(cc) main.c functions.c -o build/systembar

