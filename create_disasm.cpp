#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "llvm.hpp"

EXTERNAL void* LLVMCreateDisasmCPUFeatures(const char *tripleName, const char* cpu, const char *features, void* disInfo, int tagType, LLVMOpInfoCallback getOpInfo, LLVMSymbolLookupCallback symbolLookUp);

void* llvm_create_disasm_cpu_features(const char *tripleName, const char* cpu, const char *features, void* disInfo, int tagType, LLVMOpInfoCallback getOpInfo, LLVMSymbolLookupCallback symbolLookUp){

	return LLVMCreateDisasmCPUFeatures(tripleName, cpu, features, disInfo, tagType, getOpInfo, symbolLookUp);
}

EXTERNAL void* LLVMCreateDisasmCPU(const char *tripleName, const char* cpu, void* disInfo, int tagType, LLVMOpInfoCallback getOpInfo, LLVMSymbolLookupCallback symbolLookUp);

void* llvm_create_disasm_cpu(const char *tripleName, const char* cpu, void* disInfo, int tagType, LLVMOpInfoCallback getOpInfo, LLVMSymbolLookupCallback symbolLookUp){

	return LLVMCreateDisasmCPU(tripleName, cpu, disInfo, tagType, getOpInfo, symbolLookUp);
}

