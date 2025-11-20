cmake_minimum_required(VERSION 3.27)
project(AppSupport C)

set(CMAKE_C_STANDARD 99)
set(SQLITEPATH /home/agewert/Dev/AmigaLibs/sqlite/build-m68k-amigaos)

link_directories("${SQLITEPATH}/lib" "/opt/amigagcc/m68k-amigaos/libnix/lib/")
include_directories("${SQLITEPATH}/include")
#add_compile_options(-lnix -noixemul)
add_compile_options(-m68020)

add_executable(shelltest src/BuildingBlocks/shelltest.c src/AppSupport/environment.c
        src/AppSupport/collections.c)

add_executable(gfxtest src/BuildingBlocks/gfxtest.c)
add_executable(tilemaptest src/BuildingBlocks/tilemaptest.c)
add_executable(sqlitetest src/BuildingBlocks/sqlitetest.c
        src/BuildingBlocks/sqlitetest.c)

target_link_libraries(sqlitetest PRIVATE libsqlite3.a)
