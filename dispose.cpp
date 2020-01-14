#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "llvm-init.hpp"

EXTERNAL void LLVMDisasmDispose(void* dc);

void llvm_disasm_dispose(void* dc){
	printf("here %s\n", __func__);

	LLVMDisasmDispose(dc);
}
