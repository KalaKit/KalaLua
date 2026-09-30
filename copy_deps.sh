#!/bin/sh

# Move file for use with mf, read more at https://github.com/greeenlaser/personal-stash/tree/main/mf

set -e

#
# References
#

EXTERNAL_DIR=external

KH_ORIGIN=../kalaheaders
KH_TARGET=${EXTERNAL_DIR}/kalaheaders

LUA_ORIGIN=../../_forks/lua/_build
LUA_TARGET=${EXTERNAL_DIR}/lua

#
# Copy dependencies
#

# Always a fresh start
rm -rf "${EXTERNAL_DIR}"
mkdir "${EXTERNAL_DIR}"

# KalaHeaders
mkdir "${KH_TARGET}"

mf --f "${KH_ORIGIN}/README.md" --t "${KH_TARGET}/README.md"
mf --f "${KH_ORIGIN}/LICENSE.md" --t "${KH_TARGET}/LICENSE.md"

mf --f "${KH_ORIGIN}/include" --t "${KH_TARGET}"

# Lua
mkdir "${LUA_TARGET}"

mf --f "${LUA_ORIGIN}/LICENSE" --t "${LUA_TARGET}/LICENSE"

mf --f "${LUA_ORIGIN}/include" --t "${LUA_TARGET}"

mf --f "${LUA_ORIGIN}/release" --t "${LUA_TARGET}"
mf --f "${LUA_ORIGIN}/debug" --t "${LUA_TARGET}"
