#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define EXTERNAL extern "C"

typedef int(* LLVMOpInfoCallback) (void *DisInfo, uint64_t PC, uint64_t Offset, uint64_t Size, int TagType, void *TagBuf);
typedef const char*(* LLVMSymbolLookupCallback) (void *DisInfo, uint64_t ReferenceValue, uint64_t *ReferenceType, uint64_t ReferencePC, const char **ReferenceName);

int opinfoCallback(void *DisInfo, uint64_t pc, uint64_t offset, uint64_t size, int tagType, void *tagBuf);

const char *symbolLookupCallback(void *DisInfo, uint64_t ReferenceValue, uint64_t *ReferenceType, uint64_t ReferencePC, const char **ReferenceName);

void llvm_disas_init(void);

void* llvm_create_disasm_cpu_features(const char *tripleName, const char* cpu, const char *features, void* disInfo, int tagType, LLVMOpInfoCallback getOpInfo, LLVMSymbolLookupCallback symbolLookUp);

void* llvm_create_disasm_cpu(const char *tripleName, const char* cpu, void* disInfo, int tagType, LLVMOpInfoCallback getOpInfo, LLVMSymbolLookupCallback symbolLookUp);

int llvm_disasm_instruction(void *dc, uint8_t *bytes, uint64_t bytessize, uint64_t pc, char * outString, uint32_t strlen);

void llvm_disasm_dispose(void *dc);

