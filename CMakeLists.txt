# Older cmakes doesn't allow "generator expressions" such as $<TARGET_FILE:...> in target_link_libraries
cmake_minimum_required(VERSION 2.8.11)

project (renode-llvm-disas)

set(LLVM_VERSION	9.0.0)

add_definitions(-Wall)

add_library(LLVM SHARED init.c stub.c create_disasm.c disasm_instruction.c dispose.c)
target_link_libraries(LLVM -Wl,--unresolved-symbols=report-all)

# Use "C++ linker" to create LLVM target as LLVM code in linked libraries is written in C++
set_property(TARGET LLVM PROPERTY LINKER_LANGUAGE "CXX")

add_executable(test-app test.c)

if(CMAKE_HOST_UNIX)  # "Set to true when the host system is UNIX or UNIX like (i.e. APPLE and CYGWIN)."
	set(LLVM_LIBS_DIR	${CMAKE_SOURCE_DIR}/llvm-libs)
	set(LLVM_XZ_PATH	${CMAKE_SOURCE_DIR}/llvm-${LLVM_VERSION}.tar.xz)

	if(NOT IS_DIRECTORY ${LLVM_LIBS_DIR})
		message("Necessary LLVM libraries not found...")
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

		if(NOT EXISTS ${LLVM_XZ_PATH})
			message("Downloading LLVM...")
			file(DOWNLOAD http://releases.llvm.org/${LLVM_VERSION}/clang+llvm-${LLVM_VERSION}-x86_64-${LLVM_OS}.tar.xz ${LLVM_XZ_PATH} SHOW_PROGRESS)
		endif()

		message("Unpacking LLVM libraries...")
		execute_process(COMMAND tar xf ${LLVM_XZ_PATH} ${TAR_OS_OPTS} --strip-components=2 "clang+llvm-${LLVM_VERSION}-x86_64-${LLVM_OS}/lib/libLLVM*.a" WORKING_DIRECTORY ${LLVM_LIBS_DIR})

		file(REMOVE ${LLVM_XZ_PATH})
	endif()

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

	target_link_libraries(LLVM ${LLVM_LIBS} ${LLVM_LIBS-LAST})

	# -pthread flag while compiling (add_definitions(-pthread)) shouldn't be needed as threads are only used by libraries
	target_link_libraries(LLVM pthread)

	# To suppress CMP0042 policy warning about not setting MACOSX_RPATH for library on MAC OS X (which is only used in 'make install')
	if(CMAKE_HOST_APPLE)
		set_property(TARGET LLVM PROPERTY MACOSX_RPATH OFF)
	endif()

	# With just 'LLVM' (instead of path to libLLVM.so), Linux command to build test-app executable links all libLLVM*.a again which will succesfully create executable even if we call libLLVM*.a functions directly from test.c (which is not fine as we want our library to be independent of those libraries)
	target_link_libraries(test-app $<TARGET_FILE:LLVM>)

	# However then we have to explicitly tell cmake to build test-app after LLVM
	add_dependencies(test-app LLVM)

	# Override default "-rdynamic" linking flag (preventing adding all dynamic symbols from statically linked libraries to a resulting executable)
	set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
elseif(CMAKE_HOST_WIN32)
	set(LLVM_EXE_PATH       ${CMAKE_BINARY_DIR}/llvm-${LLVM_VERSION}-win64.exe)
	set(LLVM-C_DIR          ${CMAKE_BINARY_DIR})
	set(LLVM-C_PATH         ${LLVM-C_DIR}/LLVM-C.dll)

	if(NOT EXISTS ${LLVM-C_PATH})
		message("Necessary LLVM library not found...")

		find_program(7ZIP 7z HINTS "C:/Program Files/7-Zip/")

		if(${7ZIP} EQUAL "7ZIP-NOTFOUND")
			message(FATAL_ERROR "7-zip not found! It is required for extracting LLVM-C.dll from LLVM installer.\n7z.exe can be pointed to by adding -D 7ZIP=\"<path>\" (e.g. -D 7ZIP=\"C:/Program Files/7-Zip/7z.exe\") while invoking CMake.")
		endif()

		if(NOT EXISTS ${LLVM_EXE_PATH})
			message("Downloading LLVM...")
			file(DOWNLOAD http://releases.llvm.org/${LLVM_VERSION}/LLVM-${LLVM_VERSION}-win64.exe ${LLVM_EXE_PATH} SHOW_PROGRESS)
		endif()

		message("Unpacking LLVM library...")
		execute_process(COMMAND ${7ZIP} e ${LLVM_EXE_PATH} bin/LLVM-C.dll WORKING_DIRECTORY ${LLVM-C_DIR})

		if(EXISTS ${LLVM-C_PATH})
			file(REMOVE ${LLVM_EXE_PATH})
		else()
			message(FATAL_ERROR "Problem extracting LLVM-C.dll library out of LLVM installer!")
		endif()
	endif()

	target_link_libraries(LLVM ${LLVM-C_PATH})

	target_link_libraries(test-app LLVM)
else()
	message(FATAL_ERROR "The only supported operating systems are Windows and UNIX-like systems!")
endif()
