cmake_minimum_required(VERSION 3.27)
project(AppSupport C)

set(CMAKE_C_STANDARD 99)

add_executable(shelltest src/shelltest/shelltest.c src/AppSupport/environment.c
        src/AppSupport/collections.c)