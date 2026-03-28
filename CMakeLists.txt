cmake_minimum_required(VERSION 3.27)
project(AppSupport C)

set(CMAKE_C_STANDARD 99)
set(SQLITEPATH /home/agewert/Dev/AmigaLibs/sqlite/build-m68k-amigaos)
set(TTLIBPATH /home/agewert/Dev/AmigaLibs/ttengine-7.2/Developer/include)
set(AMISSLPATH /home/agewert/Dev/AmigaLibs/AmiSSL/Developer)

link_directories("${SQLITEPATH}/lib" "${AMISSLPATH}/lib/AmigaOS3" "/opt/amigagcc/m68k-amigaos/libnix/lib/")
include_directories("${SQLITEPATH}/include" "${AMISSLPATH}/include" ${TTLIBPATH} src)
add_compile_options(-m68020 -mcrt=clib2)

# Shell tools
add_executable(fetch src/Shelltools/fetch.c src/AppSupport/network.c src/AppSupport/collections.c)
add_executable(cldir src/Shelltools/cldir.c src/AppSupport/shell.c src/AppSupport/environment.c)
add_executable(mktemp src/Shelltools/mktemp.c src/AppSupport/filetools.c src/AppSupport/collections.c src/AppSupport/network.c)

# Tests
add_executable(archivetest src/AppSupport/archive.c src/AppSupportTests/archivetest.c src/AppSupport/collections.c src/AppSupport/testutils.c src/AppSupport/shell.c src/AppSupport/environment.c)
add_executable(fetchtest src/AppSupportTests/fetchtest.c src/AppSupport/network.c src/AppSupport/collections.c src/AppSupport/environment.c src/AppSupport/shell.c)
add_executable(shelltest src/AppSupportTests/shelltest.c src/AppSupport/environment.c src/AppSupport/collections.c src/AppSupport/filetools.c src/AppSupport/shell.c src/AppSupport/network.c)

# Building blocks
add_executable(gfxfunctions src/BuildingBlocks/gfxfunctions.c)
add_executable(sqliteclient src/BuildingBlocks/sqliteclient.c)
add_executable(tilemapfunctions src/BuildingBlocks/tilemapfunctions.c)

target_link_libraries(sqliteclient PRIVATE libsqlite3.a)
# Example for mui link lib
#target_link_libraries(Clock PRIVATE libmui.a)
