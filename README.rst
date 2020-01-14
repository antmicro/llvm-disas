llvm-disassembler
=================

Generating build with *CMake* on Windows
----------------------------------------

The only fully tested Windows *CMake* generator is ``MinGW Makefiles``.
Unfortunately, it doesn't like when ``sh.exe`` appears in *PATH* environment variable but it still worked well in case of this library after suppressing that warning.

That's why the best way to run *CMake* seems to be:

	cmake.exe [PATH_TO_SOURCE] -G "MinGW Makefiles" -DCMAKE_SH="CMAKE_SH-NOTFOUND"

and then building it using ``mingw32-make.exe``.

During the first build-files generation *CMake* downloads and extracts ``LLVM-C.dll`` library using *7-Zip*.
That's why it looks for ``7z.exe`` installed but we can specify path of ``7z.exe`` (if it's e.g. *"portable"* version of *7-Zip*) using ``-D`` switch:

	-D7ZIP="C:/Users/John Doe/Downloads/7z.exe"

After the first run and creating ``CMakeCache.txt`` in the build directory *CMake* can be run in that directory without those switches - even if it needs to use *7-Zip* again when ``LLVM-C.dll`` is missing.

