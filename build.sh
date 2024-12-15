#!/bin/bash

if [ -z "$OS" ]; then
    OS=$(uname)
fi

case "$1" in
    "install")
        if [ "$OS" != "Linux" ]; then
            echo "ERROR: \`install\` command is only available on Linux" >&2
            exit 1
        fi

        install -m 755 zoomer "${INSTALL_DIR:=/usr/local/bin}"
        exit
        ;;

    "uninstall")
        if [ "$OS" != "Linux" ]; then
            echo "ERROR: \`uninstall\` command is only available on Linux" >&2
            exit 1
        fi

        rm -f "${INSTALL_DIR:=/usr/local/bin}/zoomer"
        exit
        ;;

    *)
        if [ ! -z "$1" ]; then
            printf "ERROR: unknown command \`%s\`\n" "$1" >&2
            exit 1
        fi
        ;;
esac

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
