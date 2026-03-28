# How to build

## Requirements
These sources utilize AmigaOS 3.2 and a decent NDK for it. So don't try any older NDK or
the NDK that target AmigaOS 3.5 and AmigaOS 3.9.

Since the sources try to use standard C constructs and the ISO standard libraries only, it should
work with different compilers and cross-compilers. I tried to test different setups on Linux and AmigaOS.
You should be able to build on Windows, but I didn't test this.


## Cross compiling on Linux
I was able to setup a very well working development environment on Linux by using CLion
with CMake and the Bebbo's gcc cross-compiler. Bebbo's gcc cross-compiler is available on
https://franke.ms/git/bebbo/amiga-gcc. It supports the installation of some common thirds-party
libraries. Other libraries must be installed manually. Normally you have to extract some
archives anywhere and edit the `CMakeLists.txt` file to point to the correct location.

This setup is very powerful, so this is my usual development environment and I don't regularly
test compiling on other platforms.


## Compiling on Amiga
The included `Makefile` can be run with the latest version of GNU make which you can find
on Aminet. You may adjust some paths at the beginning of the file.
The sources should compile without any problems with a native `vbcc` version or most of the
available `gcc` ports for AmigaOS.

// TODO: Check if the sources still compile with vbcc on amiga.
