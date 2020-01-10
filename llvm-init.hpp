#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define EXTERNAL extern "C"

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

EXTERNAL int LLVMDisasmInstruction(void *dc, uint8_t *bytes, uint64_t bytessize, uint64_t pc, char * outString, uint32_t strlen);
typedef int(* LLVMOpInfoCallback) (void *DisInfo, uint64_t PC, uint64_t Offset, uint64_t Size, int TagType, void *TagBuf);
typedef const char*(* LLVMSymbolLookupCallback) (void *DisInfo, uint64_t ReferenceValue, uint64_t *ReferenceType, uint64_t ReferencePC, const char **ReferenceName);

EXTERNAL void* LLVMCreateDisasmCPUFeatures(const char *tripleName, const char* cpu, const char *features, void* disInfo, int tagType, LLVMOpInfoCallback getOpInfo, LLVMSymbolLookupCallback symbolLookUp);

int opinfoCallback(void *DisInfo, uint64_t pc, uint64_t offset, uint64_t size, int tagType, void *tagBuf);

const char *symbolLookupCallback(void *DisInfo, uint64_t ReferenceValue, uint64_t *ReferenceType, uint64_t ReferencePC, const char **ReferenceName);

void llvm_disas_init(void);
