#include <stdlib.h>

#include "llvm-disas.h"

void *LLVMCreateDisasmCPU(const char *tripleName, const char *cpu, void *disInfo, int tagType, void *getOpInfo, void *symbolLookUp);

void *llvm_create_disasm_cpu(const char *tripleName, const char *cpu)
{

    return LLVMCreateDisasmCPU(tripleName, cpu, NULL, 0, NULL, NULL);
}
