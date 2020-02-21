#include "llvm.h"

void LLVMDisasmDispose(void *dc);

void llvm_disasm_dispose(void *dc)
{

    LLVMDisasmDispose(dc);
}
