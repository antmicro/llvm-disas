#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "llvm.hpp"

int opinfoCallback(void *DisInfo, uint64_t pc, uint64_t offset, uint64_t size, int tagType, void *tagBuf) {
	printf("here %s\n", __func__);

	return 1;
}

const char *symbolLookupCallback(void *DisInfo, uint64_t ReferenceValue, uint64_t *ReferenceType, uint64_t ReferencePC, const char **ReferenceName) {
        printf("here %s\n", __func__);

	return "unknown";
}

EXTERNAL void* LLVMCreateDisasmCPUFeatures(const char *tripleName, const char* cpu, const char *features, void* disInfo, int tagType, LLVMOpInfoCallback getOpInfo, LLVMSymbolLookupCallback symbolLookUp);

void* llvm_create_disasm_cpu_features(const char *tripleName, const char* cpu, const char *features, void* disInfo, int tagType, LLVMOpInfoCallback getOpInfo, LLVMSymbolLookupCallback symbolLookUp){
	printf("here %s\n", __func__);

	return LLVMCreateDisasmCPUFeatures(tripleName, cpu, features, disInfo, tagType, getOpInfo, symbolLookUp);
}

EXTERNAL void* LLVMCreateDisasmCPU(const char *tripleName, const char* cpu, void* disInfo, int tagType, LLVMOpInfoCallback getOpInfo, LLVMSymbolLookupCallback symbolLookUp);

void* llvm_create_disasm_cpu(const char *tripleName, const char* cpu, void* disInfo, int tagType, LLVMOpInfoCallback getOpInfo, LLVMSymbolLookupCallback symbolLookUp){
	printf("here %s\n", __func__);

	return LLVMCreateDisasmCPU(tripleName, cpu, disInfo, tagType, getOpInfo, symbolLookUp);
}

