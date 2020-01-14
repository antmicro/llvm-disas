# Older cmakes doesn't allow "generator expressions" such as $<TARGET_FILE:...> in target_link_libraries
cmake_minimum_required(VERSION 2.8.11)

project (renode-llvm-disas)

set(LLVM_LIBS_DIR	${CMAKE_SOURCE_DIR}/llvm-libs)
set(LLVM_VERSION	9.0.0)
set(LLVM_XZ_PATH	${CMAKE_SOURCE_DIR}/llvm-${LLVM_VERSION}.tar.xz)

add_definitions(-Wall)

if(NOT CMAKE_HOST_UNIX)  # "Set to true when the host system is UNIX or UNIX like (i.e. APPLE and CYGWIN)."
	message(FATAL_ERROR "The only supported operating systems are UNIX-like systems!")
endif()

if(NOT IS_DIRECTORY ${LLVM_LIBS_DIR})
	file(MAKE_DIRECTORY ${LLVM_LIBS_DIR})

	if(CMAKE_HOST_APPLE)
		set(LLVM_OS     darwin-apple)
		# macOS's tar doesn't need --xz and --wildcards (using --wildcards even causes error)
		set(TAR_OS_OPTS	"")
	else()
		set(LLVM_OS     pc-linux-gnu)
		# It can't be like "--xz --wildcards": cmake then uses such string as single switch
		set(TAR_OS_OPTS	--xz --wildcards)
	endif()

	file(DOWNLOAD http://releases.llvm.org/${LLVM_VERSION}/clang+llvm-${LLVM_VERSION}-x86_64-${LLVM_OS}.tar.xz ${LLVM_XZ_PATH} SHOW_PROGRESS)

	execute_process(COMMAND tar xf ${LLVM_XZ_PATH} ${TAR_OS_OPTS} --strip-components=2 "clang+llvm-${LLVM_VERSION}-x86_64-${LLVM_OS}/lib/libLLVM*.a" WORKING_DIRECTORY ${LLVM_LIBS_DIR})

	file(REMOVE ${LLVM_XZ_PATH})
endif()

add_library(llvm SHARED init.cpp stub.cpp create_disasm.cpp disasm_instruction.cpp)

# We have to add libLLVMMC.a (must be first of those!), libLLVMBinaryFormat.a and libLLVMSupport.a libraries at the end again, because linking to libraries sorted by name causes undefined dependencies from 3 libraries:
# 	libLLVMPowerPCDesc.a depends on ...MC.a
#	...MC.a depends on both ...BinaryFormat.a and ...Support.a
#
# But it bugs out when target_link_libraries stumbles upon the library from LLVM_LIBS list (duplicates around 100 libraries while linking)!

## The only way that works is to remove those libraries from list and just link those after the others
file(GLOB LLVM_LIBS ${LLVM_LIBS_DIR}/*)
set(LLVM_LIBS-LAST ${LLVM_LIBS_DIR}/libLLVMMC.a ${LLVM_LIBS_DIR}/libLLVMBinaryFormat.a ${LLVM_LIBS_DIR}/libLLVMSupport.a)
list(REMOVE_ITEM LLVM_LIBS ${LLVM_LIBS-LAST})

# Order of linked libraries is important. Globbed files list is sorted implicitly only since cmake 3.10.
list(SORT LLVM_LIBS)

target_link_libraries(llvm ${LLVM_LIBS} ${LLVM_LIBS-LAST})

# -pthread flag while compiling (add_definitions(-pthread)) shouldn't be needed as threads are only used by libraries
target_link_libraries(llvm pthread)

add_executable(test-app test.cpp)

# With just 'llvm' (instead of path to libllvm.so), command to build test-app executable links all libLLVM*.a again which will succesfully link even if we call libLLVM*.a functions directly from test.cpp (which is not fine as we want our library to be fully sufficient)
target_link_libraries(test-app $<TARGET_FILE:llvm>)

# However then we have to explicitly tell cmake to build test-app after llvm
add_dependencies(test-app llvm)

# Override default "-rdynamic" linking flag (preventing adding all dynamic symbols from statically linked libraries to a resulting executable)
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
