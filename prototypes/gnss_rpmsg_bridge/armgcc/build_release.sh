#!/bin/sh

set -e

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SDK_ROOT="$HOME/docker-containers/var-fsl-yocto/sources/vendor/freertos-variscite"
BUILD_DIR="$SCRIPT_DIR/release"

if [ ! -f "$SDK_ROOT/tools/cmake_toolchain_files/armgcc.cmake" ]; then
    echo "Missing toolchain file:"
    echo "$SDK_ROOT/tools/cmake_toolchain_files/armgcc.cmake"
    exit 1
fi

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

cd "$BUILD_DIR"

cmake \
    -G "Unix Makefiles" \
    -DCMAKE_TOOLCHAIN_FILE="$SDK_ROOT/tools/cmake_toolchain_files/armgcc.cmake" \
    -DSdkRootDirPath="$SDK_ROOT" \
    -DCMAKE_BUILD_TYPE=release \
    "$SCRIPT_DIR"

make -j"$(nproc)"