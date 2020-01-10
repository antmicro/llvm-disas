#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "llvm-init.hpp"

#define INIT_ARCH(x) LLVMInitialize ## x ## Disassembler(); \
                     LLVMInitialize ## x ## TargetInfo(); \
                     LLVMInitialize ## x ## TargetMC();

int opinfoCallback(void *DisInfo, uint64_t pc, uint64_t offset, uint64_t size, int tagType, void *tagBuf) {
	printf("here %s\n", __func__);
	return 1;
}

const char *symbolLookupCallback(void *DisInfo, uint64_t ReferenceValue, uint64_t *ReferenceType, uint64_t ReferencePC, const char **ReferenceName) {
        printf("here %s\n", __func__);
	return "unknown";
}

void llvm_disas_init()
{
	INIT_ARCH(ARM);
	INIT_ARCH(RISCV);
	INIT_ARCH(X86);
	INIT_ARCH(AArch64);
	INIT_ARCH(Sparc);
	INIT_ARCH(PowerPC);
	INIT_ARCH(Mips);
}

