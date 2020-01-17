llvm-disassembler
=================

Generating build with *CMake* on Windows
----------------------------------------

The only fully tested Windows *CMake* generator is *"MinGW Makefiles"*.
To run *CMake* specifying generator, run it with ``-G [GENERATOR]`` flag in a clean build directory (after the first run and creation of ``CMakeCache.txt`` generator can't be changed):

	cmake.exe [PATH_TO_SOURCE] -G "MinGW Makefiles"

and then build it using ``mingw32-make.exe``.

During the first build-files generation *CMake* downloads and extracts ``LLVM-C.dll`` library using *7-Zip*.
That's why it looks for ``7z.exe`` installed but we can specify path of ``7z.exe`` (if it's e.g. *"portable"* version of *7-Zip*) using ``-D`` switch:

	-D7ZIP="C:/Users/John Doe/Downloads/7z.exe"

After the first run and creating ``CMakeCache.txt`` in the build directory *CMake* can be run in that directory without those switches - even if it needs to use *7-Zip* again when ``LLVM-C.dll`` is missing.

Generating *"MinGW Makefiles"* with *sh.exe* in *PATH* (e.g. with *Cygwin* installed)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Unfortunately, *"MinGW Makefiles"* generator doesn't like when ``sh.exe`` appears in *PATH* environment variable.
However, generation of *"MinGW Makefiles"* worked well during testing after suppressing that warning.
So in case of encountering such an error::

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

please try using *CMake* with an additional ``-DCMAKE_SH="CMAKE_SH-NOTFOUND`` switch.
It's only needed for the first run of *CMake*.
