cmake_minimum_required(VERSION 2.8)
include (ExternalProject)

project (llvm-disas)

set(LLVM_LIBS_DIR ${CMAKE_SOURCE_DIR}/llvm-libs)

# TODO check if all libraries are available and maybe even specify version of llvm-libs?
if(NOT EXISTS ${LLVM_LIBS_DIR})
	file(MAKE_DIRECTORY ${LLVM_LIBS_DIR})

	# TODO Perhaps add_custom_command would be better but how to set OUTPUT for 140 files to use then as compilation libraries?
	#
	# add_custom_command(OUTPUT LLVM_LIBS COMMAND wget -c http://releases.llvm.org/9.0.0/clang+llvm-9.0.0-x86_64-pc-linux-gnu.tar.xz -O - | tar x --xz --wildcards --strip-components=2 "clang+llvm-9.0.0-x86_64-pc-linux-gnu/lib/libLLVM*.a" WORKING_DIRECTORY ${LLVM_LIBS_DIR})

	# TODO wget's regular progress bar is not in-place, perhaps we could just quiet it? For now I'll keep it dot:giga to see any progress

	# TODO CHECK OPERATING SYSTEM TO KNOW WHICH LIBS TO DOWNLOAD
	# OS X url is http://releases.llvm.org/9.0.0/clang%2bllvm-9.0.0-x86_64-darwin-apple.tar.xz
	# Win64 url is http://releases.llvm.org/9.0.0/LLVM-9.0.0-win64.exe (but there's also win32.exe)

	execute_process(COMMAND wget --progress=dot:giga http://releases.llvm.org/9.0.0/clang+llvm-9.0.0-x86_64-pc-linux-gnu.tar.xz -O - COMMAND tar x --xz --wildcards --strip-components=2 "clang+llvm-9.0.0-x86_64-pc-linux-gnu/lib/libLLVM*.a" WORKING_DIRECTORY ${LLVM_LIBS_DIR})
endif()

add_executable(main main.cpp stub.cpp)

file(GLOB LLVM_LIBS ${LLVM_LIBS_DIR}/*)

# We have to add libLLVMMC.a (must be first of those!), libLLVMBinaryFormat.a and libLLVMSupport.a libraries at the end again, because linking to libraries sorted by name causes undefined dependencies from 3 libraries:
# 	libLLVMPowerPCDesc.a depends on ...MC.a
#	...MC.a depends on both ...BinaryFormat.a and ...Support.a
#
# But it bugs out when target_link_libraries stumbles upon the library from LLVM_LIBS list (duplicates around 100 libraries while linking)!

## Both this way...
#list(APPEND LLVM_LIBS "${LLVM_LIBS_DIR}/libLLVMBinaryFormat.a" "${LLVM_LIBS_DIR}/libLLVMMC.a" "${LLVM_LIBS_DIR}/libLLVMSupport.a")
#target_link_libraries(main PRIVATE ${LLVM_LIBS})

## ...as well as this way.
#target_link_libraries(main PRIVATE "${LLVM_LIBS}")
#target_link_libraries(main PRIVATE "${LLVM_LIBS_DIR}/libLLVMBinaryFormat.a" "${LLVM_LIBS_DIR}/libLLVMMC.a" "${LLVM_LIBS_DIR}/libLLVMSupport.a")

## The only way that works is to remove those libraries from list and just link those after the others
set(LLVM_LIBS-LAST ${LLVM_LIBS_DIR}/libLLVMMC.a ${LLVM_LIBS_DIR}/libLLVMBinaryFormat.a ${LLVM_LIBS_DIR}/libLLVMSupport.a)
list(REMOVE_ITEM LLVM_LIBS ${LLVM_LIBS-LAST})

target_link_libraries(main PRIVATE ${LLVM_LIBS} ${LLVM_LIBS-LAST})

# -pthread has meaning for both preprocessor and linker
add_definitions(-pthread)
target_link_libraries(main PRIVATE pthread)

target_link_libraries(main PRIVATE -static-libgcc -static-libstdc++)

# Override default "-rdynamic" linking flag (preventing adding all dynamic symbols from statically linked libraries to a resulting executable)
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
