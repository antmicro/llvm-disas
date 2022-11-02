#
# Copyright (c) 2019-2022 Antmicro
#
# This file is licensed under the Apache License 2.0.
# Full license text is available in the 'LICENSE' file.

# Because of `BUILD_RPATH_USE_ORIGIN` target property
cmake_minimum_required(VERSION 3.14)

project (renode-llvm-disas)


# Variables describing LLVM libraries

set(LLVM_SRC_URL    https://github.com/llvm/llvm-project/archive/llvmorg-14.0.0-rc1.tar.gz)
set(LLVM_SRC_SHA512 62aac6a033ef0e321aef5060dcc61eb721b115a9cc1b7570497f72183d9e70a66b5910e90df14428bf56d33a44d4582c919e19f4477c0cfb3209e249b53fe534)

set(LLVM_LIBS_DIR   ${CMAKE_SOURCE_DIR}/cache/${CMAKE_SYSTEM_PROCESSOR})
set(LLVM_LIBS       AArch64Desc AArch64Disassembler AArch64Info AArch64Utils ARMDesc ARMDisassembler ARMInfo ARMUtils BinaryFormat MC MCDisassembler MipsDesc MipsDisassembler MipsInfo PowerPCDesc PowerPCDisassembler PowerPCInfo RISCVDesc RISCVDisassembler RISCVInfo SparcDesc SparcDisassembler SparcInfo Support X86Desc X86Disassembler X86Info)

# On macOS "sys::PrintStackTrace" needs extra "itaniumDemangle" function from LLVMDemangle library
if(CMAKE_HOST_APPLE)
    list(APPEND LLVM_LIBS Demangle)
endif()

foreach(LIB ${LLVM_LIBS})
    list(APPEND LLVM_LIB_PATHS ${LLVM_LIBS_DIR}/libLLVM${LIB}.a)
endforeach(LIB)

set(BUILD_LLVM_LIBS OFF CACHE BOOL "Controls whether LLVM will be built from source; by default '${LLVM_LIBS_DIR}/libLLVM*.a' are used if all required libraries are found.")
if(NOT BUILD_LLVM_LIBS)
    # Make sure the directory contains all the required LLVM libraries; otherwise build them all.
    foreach(LIB ${LLVM_LIB_PATHS})
        if(NOT EXISTS ${LIB})
            message(WARNING "A required '${LIB}' library wasn't found! All LLVM libraries will be rebuilt.")
            set(BUILD_LLVM_LIBS ON)
            break()
        endif()
    endforeach()
endif()


# Define targets

add_definitions(-Werror)
add_definitions(-Wall)

set(LLVM_TABLEGEN "" CACHE STRING "Path to prebuilt llvm-tblgen binary to use, needed when cross-compiling")
set(LLVM_TABLEGEN_COPY_COMMAND "")

if(NOT CMAKE_HOST_WIN32)
    set(STATIC_LLVM_DISAS OFF CACHE BOOL "Controls whether llvm-disas will be a static library; by default it's a shared library")
    if(NOT CMAKE_CROSSCOMPILING)
        set(LLVM_TABLEGEN_COPY_COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/llvm-ep-prefix/src/llvm-ep-build/bin/llvm-tblgen ${CMAKE_BINARY_DIR})
    endif()
endif()

if(STATIC_LLVM_DISAS)
    # LLVM objects' paths are specified after extracting them from the LLVM libraries
    add_library(llvm_objs OBJECT IMPORTED)
    add_library(llvm-disas STATIC init.c stub.c create_disasm.c disasm_instruction.c dispose.c $<TARGET_OBJECTS:llvm_objs>)

    set(TARGET_LINKING_LLVM_LIBS test-app)
else()
    add_library(llvm-disas SHARED init.c stub.c create_disasm.c disasm_instruction.c dispose.c)
    target_link_libraries(llvm-disas PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wl,--unresolved-symbols=report-all>
        $<$<CXX_COMPILER_ID:GNU>:-static-libgcc>
        $<$<CXX_COMPILER_ID:GNU>:-static-libstdc++>
        # In case it isn't passed to the linker by default (e.g. g++ v8.3.0-6 on Debian 10)
        $<$<CXX_COMPILER_ID:GNU>:-Wl,--as-needed>
    )

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
# Search libraries in a runtime directory (ELFs `RPATH` is set to '$ORIGIN')
set_target_properties(test-app PROPERTIES BUILD_RPATH_USE_ORIGIN TRUE)

# Use "C++ linker" to link target as LLVM code is written in C++
set_property(TARGET ${TARGET_LINKING_LLVM_LIBS} PROPERTY LINKER_LANGUAGE "CXX")

# LLVM libraries use POSIX threads
target_link_libraries(${TARGET_LINKING_LLVM_LIBS} PRIVATE pthread)

# Link OpenLibm on Linux instead of libm required by LLVM code
if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Linux)
    target_link_libraries(${TARGET_LINKING_LLVM_LIBS} PRIVATE ${CMAKE_SOURCE_DIR}/lib/openlibm/libopenlibm-Linux-${CMAKE_SYSTEM_PROCESSOR}.a)
endif()


# Build LLVM libraries

if(BUILD_LLVM_LIBS)
    # It doesn't fail if the directory exists.
    file(MAKE_DIRECTORY ${LLVM_LIBS_DIR})

    foreach(LIB ${LLVM_LIBS})
        list(APPEND LLVM_MAKE_TARGETS LLVM${LIB})
        list(APPEND LLVM_BUILD_PATHS  ${CMAKE_BINARY_DIR}/llvm-ep-prefix/src/llvm-ep-build/lib/libLLVM${LIB}.a)
    endforeach(LIB)

    if(LLVM_TABLEGEN)
        set(LLVM_TABLEGEN_OPTION "-DLLVM_TABLEGEN=${LLVM_TABLEGEN}")
    else()
        set(LLVM_TABLEGEN_OPTION "")
    endif()

    if("${LLVM_SRC_URL}" MATCHES "releases.*llvm-[0-9]")
        # The 'llvm-$VERSION' source archives published with a release have 'llvm' contents only.
        set(LLVM_SRC_SUBDIR  .)
        set(LLVM_PATCH_LEVEL 2)
    else()
        # GH repository archives for 'llvm-project' contain clang, llvm etc. directories.
        set(LLVM_SRC_SUBDIR  llvm)
        set(LLVM_PATCH_LEVEL 1)
    endif()

    include(ExternalProject)
    ExternalProject_Add(llvm-ep
        DOWNLOAD_DIR    ${CMAKE_SOURCE_DIR}/cache
        URL             ${LLVM_SRC_URL}
        URL_HASH        SHA512=${LLVM_SRC_SHA512}
        SOURCE_SUBDIR   ${LLVM_SRC_SUBDIR}
        PATCH_COMMAND   sh -c "find ${CMAKE_CURRENT_LIST_DIR}/llvm-patches -name '*.patch' | while read -r p $<SEMICOLON> do patch -p${LLVM_PATCH_LEVEL} < \"$p\" $<SEMICOLON> done"
        CMAKE_ARGS      "-DCMAKE_SH=${CMAKE_SH}" "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}" "-DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}" "${LLVM_TABLEGEN_OPTION}"
        # The jobserver environment for parallel building is only passed properly if '$(MAKE)' is used.
        BUILD_COMMAND   "$(MAKE)" ${LLVM_MAKE_TARGETS}
        COMMAND         ${CMAKE_COMMAND} -E copy_if_different ${LLVM_BUILD_PATHS} ${LLVM_LIBS_DIR}
        COMMAND         ${LLVM_TABLEGEN_COPY_COMMAND}
        INSTALL_COMMAND ""
        TEST_COMMAND    ""
    )
    
    add_dependencies(llvm-disas llvm-ep)
endif()


# Extract LLVM objects to combine into a single static library

if(STATIC_LLVM_DISAS)
    set(LLVM_OBJS_DIR ${CMAKE_BINARY_DIR}/obj)

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

    # Allows different whitespace characters in place of SPACEs.
    # Typically TABs are returned but Sparc and PPC sometimes return SPACEs.
    #
    # Also allows specifying multiline results in a single line.
    string(REGEX REPLACE " +" "[\ \t\n]*" RES_ESC ${RES_ESC})

    set_tests_properties(${TEST_NAME} PROPERTIES PASS_REGULAR_EXPRESSION "^\t${RES_ESC}\n$")
endfunction(disas)


set(ARCH riscv32 generic-rv32) # rv32i
disas(${ARCH} 1023002300020493  "mv     s1, tp          sb      sp, 256(t1)"    )
disas(${ARCH} 0002049340848433  "sub    s0, s1, s0      mv      s1, tp"         )
disas(${ARCH} 2049340848433     "sub    s0, s1, s0"             )
disas(${ARCH} 40848433          "sub    s0, s1, s0"             )
disas(${ARCH} 01212823          "sw     s2, 16(sp)"             )
disas(${ARCH} 00b785b3          "add    a1, a5, a1"             )
disas(${ARCH} 00008297          "auipc  t0, 8"                  )
# Zifencei and Zicsr extensions don't need to be enabled
disas(${ARCH} 0000100F          "fence.i        "               ) # Zifencei
disas(${ARCH} 3401f173          "csrrci sp, mscratch, 3"        ) # Zicsr

set(ARCH riscv32 rv32imafdc) # The same as rv32gc
disas(${ARCH} 02ab5c33          "divu           s8, s6, a0"     ) # rv32m
disas(${ARCH} 0805252f          "amoswap.w      a0, zero, (a0)" ) # rv32a
disas(${ARCH} 580123d3          "fsqrt.s        ft7, ft2, rdn"  ) # rv32f
disas(${ARCH} 5a0123d3          "fsqrt.d        ft7, ft2, rdn"  ) # rv32d
disas(${ARCH} eabc              "fsw            fa5, 80(a3)"    ) # rv32c

set(ARCH riscv32 rv32gc) # The same as rv32imafdc
disas(${ARCH} 02ab5c33          "divu           s8, s6, a0"     ) # rv32m
disas(${ARCH} 0805252f          "amoswap.w      a0, zero, (a0)" ) # rv32a
disas(${ARCH} 580123d3          "fsqrt.s        ft7, ft2, rdn"  ) # rv32f
disas(${ARCH} 5a0123d3          "fsqrt.d        ft7, ft2, rdn"  ) # rv32d
disas(${ARCH} eabc              "fsw            fa5, 80(a3)"    ) # rv32c

set(ARCH riscv32 rv32v)
disas(${ARCH} 76123057          "vmsle.vi       v0, v1, 4"      )
disas(${ARCH} 0c367057          "vsetvli        zero, a2, e8, m8, ta, ma" )
disas(${ARCH} 03058407          "vle8ff.v       v8, (a1)"       )
disas(${ARCH} 628030d7          "vmseq.vi       v1, v8, 0"      )
disas(${ARCH} 4218a757          "vfirst.m       a4, v1"         )
disas(${ARCH} 5210a057          "vmsbf.m        v0, v1"         )

set(ARCH riscv64 generic-rv64) # rv64i
disas(${ARCH} 000a3a83          "ld     s5, 0(s4)"              )
disas(${ARCH} abcd8a9b          "addiw  s5, s11, -1348"         )

set(ARCH riscv64 rv64imafdc) # The same as rv64gc
disas(${ARCH} 02ab7c3b          "remuw          s8, s6, a0"     ) # rv64m
disas(${ARCH} e705b52f          "amomaxu.d.aqrl a0, a6, (a1)"   ) # rv64a
disas(${ARCH} d0312353          "fcvt.s.lu      ft6, sp, rdn"   ) # rv64f
disas(${ARCH} d2312353          "fcvt.d.lu      ft6, sp, rdn"   ) # rv64d
disas(${ARCH} eabc              "sd             a5, 80(a3)"     ) # rv64c

set(ARCH riscv64 rv64gc) # The same as rv64imafdc
disas(${ARCH} 02ab7c3b          "remuw          s8, s6, a0"     ) # rv64m
disas(${ARCH} e705b52f          "amomaxu.d.aqrl a0, a6, (a1)"   ) # rv64a
disas(${ARCH} d0312353          "fcvt.s.lu      ft6, sp, rdn"   ) # rv64f
disas(${ARCH} d2312353          "fcvt.d.lu      ft6, sp, rdn"   ) # rv64d
disas(${ARCH} eabc              "sd             a5, 80(a3)"     ) # rv64c

set(ARCH riscv64 rv64v)
disas(${ARCH} 76123057          "vmsle.vi       v0, v1, 4"      )
disas(${ARCH} 0c367057          "vsetvli        zero, a2, e8, m8, ta, ma" )
disas(${ARCH} 03058407          "vle8ff.v       v8, (a1)"       )
disas(${ARCH} 628030d7          "vmseq.vi       v1, v8, 0"      )
disas(${ARCH} 4218a757          "vfirst.m       a4, v1"         )
disas(${ARCH} 5210a057          "vmsbf.m        v0, v1"         )

set(ARCH thumb cortex-m4)
disas(${ARCH} 0403f023440b43d1  "mvns   r1, r2          add     r3, r1          bic     r4, r3, #3"     )
disas(${ARCH} 200f2c2           "movt   r2, #8192"              )
disas(${ARCH} 58c8              "ldr    r0, [r1, r3]"           )
disas(${ARCH} 426df44f          "mov.w  r2, #60672"             )
disas(${ARCH} 10b6              "asrs   r6, r6, #2"             )

set(ARCH arm arm926ej-s)
disas(${ARCH} 32855001          "addlo  r5, r5, #1"             )
disas(${ARCH} e1b00a00          "lsls   r0, r0, #20"            )
disas(${ARCH} 1a00000a          "bne    #40"                    )

set(ARCH i386 i386)
disas(${ARCH} 140c7b6b          "imull  $20, 12(%ebx), %edi"    )
disas(${ARCH} 45                "incl   %ebp"                   )
disas(${ARCH} c0b70f            "movzwl %ax, %eax"              )
disas(${ARCH} 00fda916c50c8966  "movw   %cx, 16623894(,%eax,8)" )
disas(${ARCH} 00fc005e1d010f    "lidtl  16515166"               )

set(ARCH sparc leon3)
disas(${ARCH} 18a0e885          "restore        %g2, 24, %g2"   )
disas(${ARCH} 00000001          "nop"                           )
disas(${ARCH} 47006810          "ba     %xcc, 71"               )
disas(${ARCH} 0000d881          "flush %g0"                     )
disas(${ARCH} 0020d881          "flush 0"                       )
disas(${ARCH} 0040d881          "flush %g1"                     )
disas(${ARCH} 2060d881          "flush %g1+32"                  )
disas(${ARCH} 1c40d881          "flush %g1+%i4"                 )

set(ARCH ppc ppc32)
disas(${ARCH} 7c000048          "b      .+124"                  )
disas(${ARCH} 4000887f          "cmplw  7, 8, 0"                )
disas(${ARCH} 3400e47c          "cntlzw 4, 7"                   )
