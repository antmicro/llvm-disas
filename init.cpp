#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "llvm.hpp"

#define ADD_ARCH(x) EXTERNAL void LLVMInitialize ## x ## Disassembler(); \
                    EXTERNAL void LLVMInitialize ## x ## TargetInfo(); \
                    EXTERNAL void LLVMInitialize ## x ## TargetMC();

ADD_ARCH(ARM);
ADD_ARCH(RISCV);
ADD_ARCH(X86);
ADD_ARCH(AArch64);
ADD_ARCH(Sparc);
ADD_ARCH(PowerPC);
ADD_ARCH(Mips);

#define INIT_ARCH(x) LLVMInitialize ## x ## Disassembler(); \
                     LLVMInitialize ## x ## TargetInfo(); \
                     LLVMInitialize ## x ## TargetMC();

void llvm_disasm_init(void)
{
	INIT_ARCH(ARM);
	INIT_ARCH(RISCV);
	INIT_ARCH(X86);
	INIT_ARCH(AArch64);
	INIT_ARCH(Sparc);
	INIT_ARCH(PowerPC);
	INIT_ARCH(Mips);
}

