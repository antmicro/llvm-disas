#!/bin/bash

set -e
set -x

# Add 7-Zip's directory to the shell's PATH
PATH="/cygdrive/c/Program Files/7-Zip:$PATH"

# Download, extract and add to the shell's PATH: CMake and MinGW with POSIX (!) threads
curl -O -L 'https://github.com/Kitware/CMake/releases/download/v3.17.3/cmake-3.17.3-win64-x64.zip'
curl -O -L 'https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/mingw-builds/8.1.0/threads-posix/sjlj/x86_64-8.1.0-release-posix-sjlj-rt_v6-rev0.7z'
7z x 'cmake-3.17.3-win64-x64.zip'
7z x 'x86_64-8.1.0-release-posix-sjlj-rt_v6-rev0.7z'
PATH="$PWD/mingw64/bin:$PWD/cmake-3.17.3-win64-x64/bin:$PATH"

# LLVM requires Python 3.6+. Let's test with 'f-strings' introduced in Python 3.6.
if ! python -c 'f"test"'; then
    # Add the first directory from 'Program Files' found with Python v3.6+ to PATH.
    for _python_dir in "$(find '/cygdrive/c/Program Files' -name 'Python3*')"; do
        if "$_python_dir/python.exe" -c 'f"test"'; then
            export PATH="$_python_dir:$PATH"
            break
        fi
    done

    # Check if 'python' now calls at least Python 3.6.
    if ! python -c 'f"test"'; then
        echo "Python 3.6+ not found!"
        exit 1
    fi
fi
python --version

# Configure, build and test
mkdir shared
cd shared
cmake .. -G 'MinGW Makefiles' -DCMAKE_BUILD_TYPE=Release -DBUILD_LLVM_LIBS=1 -DCMAKE_SH=CMAKE_SH-NOTFOUND
mingw32-make VERBOSE=1

ctest -V
