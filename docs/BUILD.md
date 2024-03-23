# How to build

## Requirements
These sources utilize AmigaOS 3.2 and a decent NDK for it. So don't try any older NDK or
the NDK that target AmigaOS 3.5 and AmigaOS 3.9.

Since the sources try to use standard C constructs and the ISO standard libraries only, it should
work with different compilers and cross-compilers. I tried to test different setups on Linux and AmigaOS.
You should be able to build on Windows, but I didn't test this.

## Included applications
[...]

## Cross compiling on Linux
[...]

## Compiling on Amiga
The included `Makefile` can be run with the latest version of GNU make which you can find
on Aminet. You may adjust some paths at the beginning of the file.
The sources should compile without any problems with a native `vbcc` version or most of the
available `gcc` ports for AmigaOS.

## Other setups
[...]

## IDE support on Linux
I tried to setup different development and cross compiling environments on my Linux PC.
There are two setups that worked more or less okay-ish:

- Visual Studio Code with GNU Makefiles for `vbcc` and `gcc`
- JetBrains CLion with cmake and Bebbos's gcc cross-compiler

I included the IDE specific configuration files in the git repository though I normally
don't think that's a good idea. Maybe these files can be useful for you to setup your own
environment.

Note that before creating a project in CLion you should delete the included `Makefile` since CLion
won't enable CMake as long as it detects the `Makefile` in the project root.
