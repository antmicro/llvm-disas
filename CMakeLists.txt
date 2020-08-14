# Older CMakes have problems downloading LLVM archives
cmake_minimum_required(VERSION 3.2)

project (renode-llvm-disas)


# Variables describing LLVM libraries

set(LLVM_VERSION	9.0.0)
set(LLVM_LIBS_DIR	${CMAKE_SOURCE_DIR}/lib/llvm)
set(LLVM_LIBS		AArch64Desc AArch64Disassembler AArch64Info AArch64Utils ARMDesc ARMDisassembler ARMInfo ARMUtils BinaryFormat MC MCDisassembler MipsDesc MipsDisassembler MipsInfo PowerPCDesc PowerPCDisassembler PowerPCInfo RISCVDesc RISCVDisassembler RISCVInfo RISCVUtils SparcDesc SparcDisassembler SparcInfo Support X86Desc X86Disassembler X86Info X86Utils)

# On macOS "sys::PrintStackTrace" needs extra "itaniumDemangle" function from LLVMDemangle library
if(CMAKE_HOST_APPLE)
	list(APPEND LLVM_LIBS Demangle)
endif()

foreach(LIB ${LLVM_LIBS})
	list(APPEND LLVM_LIB_PATHS ${LLVM_LIBS_DIR}/libLLVM${LIB}.a)
endforeach(LIB)


# Define targets

add_definitions(-Werror)
add_definitions(-Wall)

if(NOT CMAKE_HOST_WIN32)
	set(STATIC_LLVM_DISAS OFF CACHE BOOL "Controls whether llvm-disas will be a static library; by default it's a shared library")
endif()

if(STATIC_LLVM_DISAS)
	# Because of using the OBJECT IMPORTED library
	cmake_minimum_required(VERSION 3.9)

	# LLVM objects' paths are specified after extracting them from the LLVM libraries
	add_library(llvm_objs OBJECT IMPORTED)
	add_library(llvm-disas STATIC init.c stub.c create_disasm.c disasm_instruction.c dispose.c $<TARGET_OBJECTS:llvm_objs>)

	set(TARGET_LINKING_LLVM_LIBS test-app)
else()
	add_library(llvm-disas SHARED init.c stub.c create_disasm.c disasm_instruction.c dispose.c)
	target_link_libraries(llvm-disas PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wl,--unresolved-symbols=report-all>)

	# PRIVATE to make sure llvm-disas contains everything. With the default PUBLIC, LLVM libraries are propagated to the test-app linkage.
	target_link_libraries(llvm-disas PRIVATE
		# LLVM libraries have some dependencies between themselves
		# GNU linker requires grouping such libraries to search them again as long as new symbols emerge and remain unresolved
		$<$<CXX_COMPILER_ID:GNU>:-Wl,--start-group>
		${LLVM_LIB_PATHS}
		$<$<CXX_COMPILER_ID:GNU>:-Wl,--end-group>
	)

	# Make sure MinGW links all non-Windows libraries statically
	target_link_libraries(llvm-disas PRIVATE $<$<BOOL:${CMAKE_HOST_WIN32}>:-static>)

	# Suppress CMP0042 policy warning about not setting MACOSX_RPATH for library on MAC OS X (which is only used in 'make install')
	if(CMAKE_HOST_APPLE)
		set_property(TARGET llvm-disas PROPERTY MACOSX_RPATH OFF)
	endif()

	set(TARGET_LINKING_LLVM_LIBS llvm-disas)
endif()

add_executable(test-app test.c)
target_link_libraries(test-app PRIVATE llvm-disas)

# Use "C++ linker" to link target as LLVM code is written in C++
set_property(TARGET ${TARGET_LINKING_LLVM_LIBS} PROPERTY LINKER_LANGUAGE "CXX")

# LLVM libraries use POSIX threads
target_link_libraries(${TARGET_LINKING_LLVM_LIBS} PRIVATE pthread)


# Preparing LLVM libraries

if(CMAKE_HOST_UNIX)  # "Set to true when the host system is UNIX or UNIX like (i.e. APPLE and CYGWIN)."
	set(LLVM_XZ_PATH	${CMAKE_SOURCE_DIR}/llvm-${LLVM_VERSION}.tar.xz)

	if(NOT IS_DIRECTORY ${LLVM_LIBS_DIR})
		message("Necessary LLVM libraries not found...")
		file(MAKE_DIRECTORY ${LLVM_LIBS_DIR})

		if(CMAKE_HOST_APPLE)
			set(LLVM_OS     darwin-apple)
			# macOS's tar default behaviour is like on Linux with --wildcards (and using --wildcards causes error)
			set(TAR_OS_OPTS	"")
		else()
			set(LLVM_OS     pc-linux-gnu)
			set(TAR_OS_OPTS	"--wildcards")
		endif()

		if(NOT EXISTS ${LLVM_XZ_PATH})
			message("Downloading LLVM...")
			file(DOWNLOAD http://releases.llvm.org/${LLVM_VERSION}/clang+llvm-${LLVM_VERSION}-x86_64-${LLVM_OS}.tar.xz ${LLVM_XZ_PATH} SHOW_PROGRESS)
		else()
			message("Unpacking LLVM from a previously downloaded ${LLVM_XZ_PATH}")
		endif()

		message("Unpacking LLVM libraries...")
		execute_process(COMMAND tar xf ${LLVM_XZ_PATH} ${TAR_OS_OPTS} --strip-components=2 "clang+llvm-${LLVM_VERSION}-x86_64-${LLVM_OS}/lib/libLLVM*.a" WORKING_DIRECTORY ${LLVM_LIBS_DIR})

		file(REMOVE ${LLVM_XZ_PATH})
	else()
		message("Using LLVM from ${LLVM_LIBS_DIR} directory")
	endif()
elseif(CMAKE_HOST_WIN32)
	foreach(LIB ${LLVM_LIB_PATHS})
		if(NOT EXISTS ${LIB})
			message("At least one of the 'libLLVM*.a' libraries doesn't exist!")
			set(BUILD_LLVM_LIBS	ON)
			break()
		endif()
	endforeach()

	if(BUILD_LLVM_LIBS)
		message("LLVM libraries will be created as a part of this build.")
		foreach(LIB ${LLVM_LIBS})
			list(APPEND LLVM_MAKE_TARGETS	LLVM${LIB})
			list(APPEND LLVM_BUILD_PATHS	${CMAKE_BINARY_DIR}/llvm-ep-prefix/src/llvm-ep-build/lib/libLLVM${LIB}.a)
		endforeach(LIB)

		include(ExternalProject)
		ExternalProject_Add(llvm-ep
			DOWNLOAD_DIR		${LLVM_LIBS_DIR}
			URL			https://releases.llvm.org/9.0.0/llvm-9.0.0.src.tar.xz
			URL_HASH		SHA512=1bb3341e1d231559b948f1505b33c2e2e03989f9b8bbfef0e0cdaff5ac43f85574c9ec5ac53399b914f497d6899425d861411024e8d7e1d1a338c1c6951ac658
			CMAKE_ARGS		"-DCMAKE_SH=${CMAKE_SH}" "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
			BUILD_COMMAND		${CMAKE_MAKE_PROGRAM} ${LLVM_MAKE_TARGETS}
			COMMAND			${CMAKE_COMMAND} -E copy_if_different ${LLVM_BUILD_PATHS} ${LLVM_LIBS_DIR}
			INSTALL_COMMAND		""
			TEST_COMMAND		""
		)

		add_dependencies(llvm-disas llvm-ep)
	endif()
else()
	message(FATAL_ERROR "The only supported operating systems are Windows and UNIX-like systems!")
endif()


# Extract LLVM objects to combine into the one static library

if(STATIC_LLVM_DISAS)
	set(LLVM_OBJS_DIR	${CMAKE_BINARY_DIR}/obj)

	file(MAKE_DIRECTORY ${LLVM_OBJS_DIR})
	foreach(LIB ${LLVM_LIB_PATHS})
		execute_process(COMMAND ${CMAKE_AR} -x ${LIB} WORKING_DIRECTORY ${LLVM_OBJS_DIR})
	endforeach(LIB)

	file(GLOB LLVM_OBJS ${LLVM_OBJS_DIR}/*)
	set_target_properties(llvm_objs PROPERTIES IMPORTED_OBJECTS "${LLVM_OBJS}")
endif()


# CTEST

enable_testing()

add_test(NAME info COMMAND test-app)
set_tests_properties(info PROPERTIES PASS_REGULAR_EXPRESSION "^Usage:")

function(disas triple cpu code result)
	set(TEST_NAME ${triple}_${cpu}_${code})
	add_test(NAME ${TEST_NAME} COMMAND test-app ${triple} ${cpu} ${code})

	# Escapes special regex characters expected in the result
	string(REGEX REPLACE "\\$" "\\\\$" RES_ESC ${result})
	string(REGEX REPLACE "\\(" "\\\\(" RES_ESC ${RES_ESC})
	string(REGEX REPLACE "\\)" "\\\\)" RES_ESC ${RES_ESC})
	string(REGEX REPLACE "\\[" "\\\\[" RES_ESC ${RES_ESC})
	string(REGEX REPLACE "\\]" "\\\\]" RES_ESC ${RES_ESC})
	string(REGEX REPLACE "\\." "\\\\." RES_ESC ${RES_ESC})
	string(REGEX REPLACE "\\+" "\\\\+" RES_ESC ${RES_ESC})

	# Allows different whitespace characters in place of TABs
	# (for Sparc and PPC sometimes SPACE is returned instead of a TAB)
	#
	# Also allows passing next line after just 1 or 2 TABs (instead of "\n\t")
	string(REGEX REPLACE "\\\t" "[\ \t\n]*" RES_ESC ${RES_ESC})

	set_tests_properties(${TEST_NAME} PROPERTIES PASS_REGULAR_EXPRESSION "^\t${RES_ESC}\n$")
endfunction(disas)

# The result string delimeter should be TAB.

set(ARCH riscv32 generic-rv32) # rv32i
disas(${ARCH} 1023002300020493	"mv	s1, tp		sb	sp, 256(t1)"	)
disas(${ARCH} 0002049340848433	"sub	s0, s1, s0	mv	s1, tp"		)
disas(${ARCH} 2049340848433	"sub	s0, s1, s0"		)
disas(${ARCH} 40848433		"sub	s0, s1, s0"		)
disas(${ARCH} 01212823		"sw	s2, 16(sp)"		)
disas(${ARCH} 00b785b3		"add	a1, a5, a1"		)
disas(${ARCH} 00008297		"auipc	t0, 8"			)
# Zifencei and Zicsr extensions don't need to be enabled
disas(${ARCH} 0000100F		"fence.i	"		) # Zifencei
disas(${ARCH} 3401f173		"csrrci	sp, mscratch, 3"	) # Zicsr

set(ARCH riscv32 rv32imafdc) # The same as rv32gc
disas(${ARCH} 02ab5c33		"divu		s8, s6, a0"	) # rv32m
disas(${ARCH} 0805252f		"amoswap.w	a0, zero, (a0)"	) # rv32a
disas(${ARCH} 580123d3		"fsqrt.s	ft7, ft2, rdn"	) # rv32f
disas(${ARCH} 5a0123d3		"fsqrt.d	ft7, ft2, rdn"	) # rv32d
disas(${ARCH} eabc		"fsw		fa5, 80(a3)"	) # rv32c

set(ARCH riscv32 rv32gc) # The same as rv32imafdc
disas(${ARCH} 02ab5c33		"divu		s8, s6, a0"	) # rv32m
disas(${ARCH} 0805252f		"amoswap.w	a0, zero, (a0)"	) # rv32a
disas(${ARCH} 580123d3		"fsqrt.s	ft7, ft2, rdn"	) # rv32f
disas(${ARCH} 5a0123d3		"fsqrt.d	ft7, ft2, rdn"	) # rv32d
disas(${ARCH} eabc		"fsw		fa5, 80(a3)"	) # rv32c

set(ARCH riscv64 generic-rv64) # rv64i
disas(${ARCH} 000a3a83		"ld	s5, 0(s4)"		)
disas(${ARCH} abcd8a9b		"addiw	s5, s11, -1348"		)

set(ARCH riscv64 rv64imafdc) # The same as rv64gc
disas(${ARCH} 02ab7c3b		"remuw		s8, s6, a0"	) # rv64m
disas(${ARCH} e705b52f		"amomaxu.d.aqrl	a0, a6, (a1)"	) # rv64a
disas(${ARCH} d0312353		"fcvt.s.lu	ft6, sp, rdn"	) # rv64f
disas(${ARCH} d2312353		"fcvt.d.lu	ft6, sp, rdn"	) # rv64d
disas(${ARCH} eabc		"sd		a5, 80(a3)"	) # rv64c

set(ARCH riscv64 rv64gc) # The same as rv64imafdc
disas(${ARCH} 02ab7c3b		"remuw		s8, s6, a0"	) # rv64m
disas(${ARCH} e705b52f		"amomaxu.d.aqrl	a0, a6, (a1)"	) # rv64a
disas(${ARCH} d0312353		"fcvt.s.lu	ft6, sp, rdn"	) # rv64f
disas(${ARCH} d2312353		"fcvt.d.lu	ft6, sp, rdn"	) # rv64d
disas(${ARCH} eabc		"sd		a5, 80(a3)"	) # rv64c

set(ARCH thumb cortex-m4)
disas(${ARCH} 0403f023440b43d1	"mvns	r1, r2		add	r3, r1		bic	r4, r3, #3"	)
disas(${ARCH} 200f2c2		"movt	r2, #8192"		)
disas(${ARCH} 58c8		"ldr	r0, [r1, r3]"		)
disas(${ARCH} 426df44f		"mov.w	r2, #60672"		)
disas(${ARCH} 10b6		"asrs	r6, r6, #2"		)

set(ARCH arm arm926ej-s)
disas(${ARCH} 32855001		"addlo	r5, r5, #1"		)
disas(${ARCH} e1b00a00		"lsls	r0, r0, #20"		)
disas(${ARCH} 1a00000a		"bne	#40"			)

set(ARCH i386 i386)
disas(${ARCH} 140c7b6b		"imull	$20, 12(%ebx), %edi"	)
disas(${ARCH} 45		"incl	%ebp"			)
disas(${ARCH} c0b70f		"movzwl	%ax, %eax"		)
disas(${ARCH} 00fda916c50c8966	"movw	%cx, 16623894(,%eax,8)"	)
disas(${ARCH} 00fc005e1d010f	"lidtl	16515166"		)

set(ARCH sparc leon3)
disas(${ARCH} 18a0e885		"restore	%g2, 24, %g2"	)
disas(${ARCH} 00000001		"nop"				)
disas(${ARCH} 47006810		"ba	%xcc, 71"		)

set(ARCH ppc ppc32)
disas(${ARCH} 7c000048		"b	.+124"			)
disas(${ARCH} 4000887f		"cmplw	7, 8, 0"		)
disas(${ARCH} 3400e47c		"cntlzw	4, 7"			)
