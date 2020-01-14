#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "llvm.hpp"

EXTERNAL void LLVMDisasmDispose(void* dc);

void llvm_disasm_dispose(void* dc){

	LLVMDisasmDispose(dc);
}
