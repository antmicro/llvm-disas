llvm-disas
==========

This library is a wrapper for *LLVM* disassembling functions.
It is used to disassemble machine code in `Renode <https://renode.io>`_ (e.g. while logging instructions executed by the emulated device).

Not all of the CPU architectures supported by *LLVM* are supported by this library.
The following *LLVM CPU architecture groups* (e.g. *X86* groups at least *i386*, *i686* and *x86_64*) are currently supported by *libllvm-disas*.

- *AArch64*
- *ARM*
- *Mips*
- *PowerPC*
- *RISCV*
- *Sparc*
- *X86*

Support for a given CPU architecture depends on whether the appropriate ``LLVMInitialize{ARCHGROUP}*`` functions are wrapped by the library (and appropriate ``libLLVM${ARCHGROUP}*`` libraries linked during building).

Building
--------

The library uses `CMake <https://cmake.org/>`_ to prepare build files.
Builds have been tested to work properly on *Linux*, *macOS* and *Windows*.
However, there are a few caveats for building it on *Windows*.

A successful build will result in creating a library and a testing executable.

LLVM libraries
++++++++++++++

This project requires some of the ``libLLVM*.a`` libraries from the *LLVM* project.

They are built from source unless all of them are found in the ``<REPOSITORY_ROOT>/cache/<ARCH>`` directory (``<ARCH>`` is CMake's ``CMAKE_SYSTEM_PROCESSOR`` variable).
After the libraries are successfully built, they will be copied to that cache directory to preserve them for any future build.
Use the ``BUILD_LLVM_LIBS`` option to force building ``libLLVM*.a`` regardless of the cache contents::

    cmake <REPOSITORY_ROOT> -DBUILD_LLVM_LIBS=ON

The *LLVM* source archive is cached in ``<REPOSITORY_ROOT>/cache``, to prevent it from being downloaded for every *"clean"* build.

Linux and macOS
+++++++++++++++

Building this library on *Linux* and *macOS* is quite straightforward.

Building this project on both of these operating systems has been tested using *CMake*'s default *"Unix Makefiles"* generator with *make* and with the most common compiler for each of them – *GCC* on *Linux* and *AppleClang* on *macOS*.

After linking with static LLVM libraries, the result of a build is a single independent shared (dynamic) library named ``libLLVM.so`` on *Linux* and ``libLLVM.dylib`` on *macOS*.

Windows
+++++++

For now, the only fully tested Windows *CMake* generator is *"MinGW Makefiles"*, which is not the default one.
To run *CMake* specifying the generator, run it with the ``-G [GENERATOR]`` flag::

   cmake.exe [PATH_TO_SOURCE] -G "MinGW Makefiles"

and then build it using ``mingw32-make.exe``.
It has to be done in a clean build directory because after the first run and creation of ``CMakeCache.txt``, the generator can't be changed.

Generating *"MinGW Makefiles"* with *sh.exe* in *PATH* (e.g. with *Cygwin* installed)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Unfortunately, the *"MinGW Makefiles"* generator doesn't like it when ``sh.exe`` appears in the *PATH* environment variable.
However, generation of *"MinGW Makefiles"* worked well during testing after suppressing that warning.
So if you encounter such an error::

        CMake Error at {CMAKE_PATH}/Modules/CMakeMinGWFindMake.cmake:12 (message):
          sh.exe was found in your PATH, here:

          C:/cygwin64/bin/sh.exe

          For MinGW make to work correctly sh.exe must NOT be in your path.

          Run cmake from a shell that does not have sh.exe in your PATH.

          If you want to use a UNIX shell, then use MSYS Makefiles.

        Call Stack (most recent call first):
          CMakeLists.txt:4 (project)


        CMake Error: CMAKE_C_COMPILER not set, after EnableLanguage
        CMake Error: CMAKE_CXX_COMPILER not set, after EnableLanguage
        -- Configuring incomplete, errors occurred!

try using *CMake* with an additional ``-DCMAKE_SH="CMAKE_SH-NOTFOUND`` switch.
It's only needed for the first run of *CMake*.

Testing
-------

The ``test-app`` testing executable provides a way to test the ``llvm-disas`` library's disassembly capabilities.

Manual testing
++++++++++++++

``test-app`` can be executed manually with the ``{cpu-arch} {cpu-model} {block}`` arguments.
The first two arguments indicate the cpu architecture (e.g. ``riscv64``) and the cpu model (e.g. ``generic-rv64``), respectively, and are based on the *LLVM* naming.

The third argument is used to pass a block of the machine code to disassemble, represented as a string containing hexadecimal digits (doesn't have to be prefixed with ``0x``).
In a single execution, ``test-app`` can disassemble up to 8B of machine code (up to 16 hex digits), which may contain multiple instructions.

Automatic testing 
+++++++++++++++++

Automatic testing is supported by the `CTest <https://gitlab.kitware.com/cmake/community/-/wikis/doc/ctest/Testing-With-CTest>`_.
``CMakeLists.txt`` contains automatic tests, which test the disassembling of various *"code-blocks"* in a various CPU architectures supported by this library.

They can be executed after building the project using the ``ctest`` command in a build directory.
