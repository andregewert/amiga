ccbin = vc
ccopts = -c99
includes = -I${NDK_INC}
libs = -lamiga -lauto

mathlib = -lmsoft
cc = $(ccbin) $(ccopts) $(includes) $(libs)

all: shelltest

prepare:
	mkdir -p build

clean:
	rm -r build

shelltest: prepare src/AppSupportTests/shelltest.c src/AppSupport/environment.c
	$(cc) src/shelltest/shelltest.c src/AppSupport/environment.c -o build/shelltest
