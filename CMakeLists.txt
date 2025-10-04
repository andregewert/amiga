cmake_minimum_required(VERSION 3.27)
project(AppSupport C)

set(CMAKE_C_STANDARD 99)
set(SQLITEPATH /home/agewert/Dev/AmigaLibs/sqlite/build-m68k-amigaos)

link_directories("${SQLITEPATH}/lib" "/opt/amigagcc/m68k-amigaos/libnix/lib/")
include_directories("${SQLITEPATH}/include")
add_compile_options(-noixemul)

add_executable(shelltest src/shelltest/shelltest.c src/AppSupport/environment.c
        src/AppSupport/collections.c)

add_executable(gfxtest src/gfxtest/gfxtest.c)
add_executable(tilemaptest src/tilemaptest/tilemaptest.c)
add_executable(sqlitetest src/sqlitetest/sqlitetest.c
        src/sqlitetest/sqlitetest.c)

target_link_libraries(sqlitetest PRIVATE libsqlite3.a)

