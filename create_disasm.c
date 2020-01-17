#include <stdlib.h>

#include "llvm.h"

void* LLVMCreateDisasmCPUFeatures(const char *tripleName, const char* cpu, const char *features, void* disInfo, int tagType, LLVMOpInfoCallback getOpInfo, LLVMSymbolLookupCallback symbolLookUp);

void* llvm_create_disasm_cpu_features(const char *tripleName, const char* cpu, const char *features, void* disInfo, int tagType, LLVMOpInfoCallback getOpInfo, LLVMSymbolLookupCallback symbolLookUp){

	return LLVMCreateDisasmCPUFeatures(tripleName, cpu, features, disInfo, tagType, getOpInfo, symbolLookUp);
}

void* LLVMCreateDisasmCPU(const char *tripleName, const char* cpu, void* disInfo, int tagType, LLVMOpInfoCallback getOpInfo, LLVMSymbolLookupCallback symbolLookUp);

void* llvm_create_disasm_cpu(const char *tripleName, const char* cpu, void* disInfo, int tagType, LLVMOpInfoCallback getOpInfo, LLVMSymbolLookupCallback symbolLookUp){

	return LLVMCreateDisasmCPU(tripleName, cpu, disInfo, tagType, getOpInfo, symbolLookUp);
}
