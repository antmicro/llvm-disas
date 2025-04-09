#!/bin/bash

set -e
set -x

# Add 7-Zip's directory to the shell's PATH
PATH="/cygdrive/c/Program Files/7-Zip:$PATH"

# Download, extract and add to the shell's PATH: MinGW with POSIX (!) threads
curl -O -L 'https://github.com/brechtsanders/winlibs_mingw/releases/download/14.2.0posix-12.0.0-ucrt-r3/winlibs-x86_64-posix-seh-gcc-14.2.0-llvm-19.1.7-mingw-w64ucrt-12.0.0-r3.7z'
7z x 'winlibs-x86_64-posix-seh-gcc-14.2.0-llvm-19.1.7-mingw-w64ucrt-12.0.0-r3.7z'
PATH="$PWD/mingw64/bin:$PATH"

# LLVM requires Python 3.6+. Let's test with 'f-strings' introduced in Python 3.6.
if ! python -c 'f"test"'; then
    echo "Python 3.6+ not found!"
    exit 1
fi
python --version

# Configure, build and test
mkdir shared
cd shared
cmake .. -G 'MinGW Makefiles' -DCMAKE_BUILD_TYPE=Release -DBUILD_LLVM_LIBS=1 -DCMAKE_SH=CMAKE_SH-NOTFOUND -DCMAKE_TLS_VERIFY=OFF
mingw32-make -j$(nproc) VERBOSE=1

ctest -V
