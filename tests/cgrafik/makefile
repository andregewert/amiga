# Optionen für vbcc mit Target Amiga
CC = vc
CCOPT = -I${VBCC}/posix/include -I${NDK_INC} -L${VBCC}/posix/lib
LD = vc
LDOPT = -lamiga -lauto -lposix

#CC = m68k-amigaos-gcc
#CCOPT =
#LDOPT =

SOURCEDIR = src
BUILDDIR = build

prepare:
	mkdir -p build

clean:
	rm -r build

ifftest: ifftest.c prepare
	$(CC) $(CCOPT) $(LDOPT) -o $(BUILDDIR)/ifftest ifftest.c

bild: bild.c prepare
	$(CC) $(CCOPT) $(LDOPT) -o $(BUILDDIR)/bild bild.c

cccommands:
	./createCompileCommands.sh

all: ifftest bild
