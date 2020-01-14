#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "llvm.hpp"

EXTERNAL int LLVMDisasmInstruction(void *dc, uint8_t *bytes, uint64_t bytessize, uint64_t pc, char * outString, uint32_t strlen);

int llvm_disasm_instruction(void *dc, uint8_t *bytes, uint64_t bytessize, uint64_t pc, char * outString, uint32_t strlen){

	return LLVMDisasmInstruction(dc, bytes, bytessize, pc, outString, strlen);
}

