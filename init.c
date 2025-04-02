//
// Copyright (c) 2019-2024 Antmicro
//
// This file is licensed under the Apache License 2.0.
// Full license text is available in the 'LICENSE' file.
//
#include <string.h>

#include "llvm-disas.h"

#define ADD_ARCH(x)                             \
    void LLVMInitialize ## x ## Disassembler(); \
    void LLVMInitialize ## x ## TargetInfo();   \
    void LLVMInitialize ## x ## TargetMC();     \
    void LLVMInitialize ## x ## AsmParser();

ADD_ARCH(AArch64)
ADD_ARCH(ARM)
ADD_ARCH(Mips)
ADD_ARCH(PowerPC)
ADD_ARCH(RISCV)
ADD_ARCH(Sparc)
ADD_ARCH(X86)
ADD_ARCH(MSP430)

#define FUNC_INIT_ARCH(x)                      \
    void llvm_disasm_ ## x ## _init(void)      \
    {                                          \
        LLVMInitialize ## x ## Disassembler(); \
        LLVMInitialize ## x ## TargetInfo();   \
        LLVMInitialize ## x ## TargetMC();     \
        LLVMInitialize ## x ## AsmParser();    \
    }

FUNC_INIT_ARCH(AArch64)
FUNC_INIT_ARCH(ARM)
FUNC_INIT_ARCH(Mips)
FUNC_INIT_ARCH(PowerPC)
FUNC_INIT_ARCH(RISCV)
FUNC_INIT_ARCH(Sparc)
FUNC_INIT_ARCH(X86)
FUNC_INIT_ARCH(MSP430)

// Architecture aliases based on LLVM's 'parseArch' defined in lib/support/Triple.cpp
void init_llvm_architecture(const char *arch){
    if ((strncmp(arch, "arm", 3) == 0 && strncmp(arch, "arm64", 5) != 0)
                || strncmp(arch, "thumb", 5) == 0) {
        llvm_disasm_ARM_init();
    } else if (strncmp(arch, "aarch64", 7) == 0 || strncmp(arch, "arm64", 5) == 0) {
        llvm_disasm_AArch64_init();
    } else if (strncmp(arch, "mips", 4) == 0) {
        llvm_disasm_Mips_init();
    } else if (strncmp(arch, "powerpc", 7) == 0 || strncmp(arch, "ppc", 3) == 0) {
        llvm_disasm_PowerPC_init();
    } else if (strncmp(arch, "riscv", 5) == 0) {
        llvm_disasm_RISCV_init();
    } else if (strncmp(arch, "sparc", 5) == 0) {
        llvm_disasm_Sparc_init();
    } else if (strncmp(arch, "x86", 3) == 0 || strcmp(arch, "amd64") == 0
                // All i?86 from i386 to i986 are valid LLVM architectures
                || (strncmp(arch, "i", 1) == 0 && strncmp(arch+2, "86", 2) == 0)) {
        llvm_disasm_X86_init();
    } else if (strncmp(arch, "msp430", 6) == 0) {
        llvm_disasm_MSP430_init();
    }
}
