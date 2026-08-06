#!/bin/sh

# Move file for use with mf, read more at https://github.com/greeenlaser/personal-stash/tree/main/mf

set -e

#
# References
#

case "$1" in
    --linux)
        OUT_NAME=KalaLua-Linux

        LIB_NAME=libkalalua
        LIB_EXT=a
        LIB_ORIGIN=build/release-linux
        ;;
    --windows-gnu)
        OUT_NAME=KalaLua-Windows-Gnu

        LIB_NAME=kalalua-gnu
        LIB_EXT=lib
        LIB_ORIGIN=build/release-windows-gnu
        ;;
    --windows)
        OUT_NAME=KalaLua-Windows

        LIB_NAME=kalalua
        LIB_EXT=lib
        LIB_ORIGIN=build/release-windows
        ;;
    *)
        echo "Error: Argument must be --linux, --windows-gnu or --windows" >&2
        exit 1
        ;;
esac

OUT_VER=1-0-0
OUT_DIR=out/${OUT_NAME}-${OUT_VER}

README=README.md
LICENSE=LICENSE.md
INCLUDE=include
DOCS=docs

DIR_ES=../external-shared
IN_KH=${DIR_ES}/KalaHeaders
OUT_KH_NAME=kalaheaders

# Lua

IN_LUA=${DIR_ES}/lua
OUT_LUA_NAME=lua

#
# Core stuff
#

# Always a fresh start
mkdir -p "out"
rm -rf "${OUT_DIR}"
mkdir "${OUT_DIR}"
mkdir "${OUT_DIR}/${OUT_KH_NAME}"

# The base files
mf --f "${README}" --t "${OUT_DIR}/${README}"
mf --f "${LICENSE}" --t "${OUT_DIR}/${LICENSE}"
mf --f "${INCLUDE}" --t "${OUT_DIR}"
mf --f "${DOCS}" --t "${OUT_DIR}"

# The binary
if [ ! -f "${LIB_ORIGIN}/${LIB_NAME}.${LIB_EXT}" ]; then
    printf 'Error: Binary %s not found\n' "${LIB_ORIGIN}/${LIB_NAME}.${LIB_EXT}" >&2
    exit 1
fi

mf --f "${LIB_ORIGIN}/${LIB_NAME}.${LIB_EXT}" --t "${OUT_DIR}/${LIB_NAME}.${LIB_EXT}"

# KalaHeaders
mf --f "${IN_KH}/${README}" --t "${OUT_DIR}/${OUT_KH_NAME}/${README}"
mf --f "${IN_KH}/${LICENSE}" --t "${OUT_DIR}/${OUT_KH_NAME}/${LICENSE}"

mf --f "${IN_KH}/include" --t "${OUT_DIR}/${OUT_KH_NAME}"

# Lua
mkdir "${OUT_DIR}/${OUT_LUA_NAME}"

mf --f "${IN_LUA}/${README}" --t "${OUT_DIR}/${OUT_LUA_NAME}/${README}"
mf --f "${IN_LUA}/${LICENSE}" --t "${OUT_DIR}/${OUT_LUA_NAME}/${LICENSE}"

mf --f "${IN_LUA}/include" --t "${OUT_DIR}/${OUT_LUA_NAME}"

# Lua binary
case "$1" in
    --linux)
        mf --f "${IN_LUA}/release/liblua.a" --t "${OUT_DIR}/${OUT_LUA_NAME}/liblua.a"
        ;;
    --windows-gnu)
        mf --f "${IN_LUA}/release/lua-gnu.lib" --t "${OUT_DIR}/${OUT_LUA_NAME}/lua-gnu.lib"
        ;;
    --windows)
        mf --f "${IN_LUA}/release/lua.lib" --t "${OUT_DIR}/${OUT_LUA_NAME}/lua.lib"
        ;;
esac
