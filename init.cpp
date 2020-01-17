#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "llvm.h"

#define ADD_ARCH(x) void LLVMInitialize ## x ## Disassembler(); \
                    void LLVMInitialize ## x ## TargetInfo(); \
                    void LLVMInitialize ## x ## TargetMC();

ADD_ARCH(AArch64)
ADD_ARCH(ARM)
ADD_ARCH(Mips)
ADD_ARCH(PowerPC)
ADD_ARCH(RISCV)
ADD_ARCH(Sparc)
ADD_ARCH(X86)

#define FUNC_INIT_ARCH(x) void llvm_disasm_ ## x ## _init(void){ \
	LLVMInitialize ## x ## Disassembler(); \
	LLVMInitialize ## x ## TargetInfo(); \
	LLVMInitialize ## x ## TargetMC(); \
	}

FUNC_INIT_ARCH(AArch64)
FUNC_INIT_ARCH(ARM)
FUNC_INIT_ARCH(Mips)
FUNC_INIT_ARCH(PowerPC)
FUNC_INIT_ARCH(RISCV)
FUNC_INIT_ARCH(Sparc)
FUNC_INIT_ARCH(X86)
