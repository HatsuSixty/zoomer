#!/bin/sh

if [ -z "$OS" ]; then
    OS=$(uname)
fi

#==================#
#   Build Raylib   #
#==================#

pushd ./raylib-5.5/src/
make
popd

#==================#
#   Build Zoomer   #
#==================#

CFLAGS="-Wall -Wextra -std=c11 -pedantic -ggdb -I./raylib-5.5/src/ -L./raylib-5.5/src/"
LIBS="-lm -l:libraylib.a"
SOURCES="./src/main.c"

if [ "$OS" = "Windows_NT" ]; then
    SOURCES="$SOURCES ./src/screenshot_windows.c"
    LIBS="$LIBS -lgdi32 -lwinmm"
else
    # Assume Linux
    SOURCES="$SOURCES ./src/screenshot_linux.c"
fi

cc $CFLAGS -o zoomer $SOURCES $LIBS
