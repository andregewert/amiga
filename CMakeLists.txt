cmake_minimum_required(VERSION 3.27)
project(AppSupport C)

set(CMAKE_C_STANDARD 99)
set(SQLITEPATH /home/agewert/Dev/AmigaLibs/sqlite/build-m68k-amigaos)
set(TTLIBPATH /home/agewert/Dev/AmigaLibs/ttengine-7.2/Developer/include)
set(AMISSLPATH /home/agewert/Dev/AmigaLibs/AmiSSL/Developer)

link_directories("${SQLITEPATH}/lib" "${AMISSLPATH}/lib/AmigaOS3" "/opt/amigagcc/m68k-amigaos/libnix/lib/")
include_directories("${SQLITEPATH}/include" "${AMISSLPATH}/include" ${TTLIBPATH})
add_compile_options(-m68020 -mcrt=clib2)

add_executable(shelltest src/BuildingBlocks/shelltest.c src/AppSupport/environment.c src/AppSupport/collections.c src/AppSupport/filetools.c src/AppSupport/shell.c src/AppSupport/network.c)
add_executable(mktemp src/Shelltools/mktemp.c src/AppSupport/filetools.c src/AppSupport/collections.c src/AppSupport/network.c)
add_executable(fetchtest src/BuildingBlocks/fetchtest.c src/AppSupport/network.c src/AppSupport/collections.c src/AppSupport/environment.c src/AppSupport/shell.c)
add_executable(fetch src/Shelltools/fetch.c src/AppSupport/network.c src/AppSupport/collections.c)
add_executable(cldir src/Shelltools/cldir.c src/AppSupport/shell.c src/AppSupport/environment.c)
add_executable(gfxtest src/BuildingBlocks/gfxtest.c)
add_executable(tilemaptest src/BuildingBlocks/tilemaptest.c)
add_executable(sqlitetest src/BuildingBlocks/sqlitetest.c)
add_executable(Clock src/muitest/Clock.c)

target_link_libraries(sqlitetest PRIVATE libsqlite3.a)
target_link_libraries(Clock PRIVATE libmui.a)