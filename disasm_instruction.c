#include "llvm.h"

int LLVMDisasmInstruction(void *dc, uint8_t *bytes, uint64_t bytessize, uint64_t pc, char *outString, uint32_t strlen);

int llvm_disasm_instruction(void *dc, uint8_t *bytes, uint64_t bytessize, char *outString, uint32_t strlen)
{

    return LLVMDisasmInstruction(dc, bytes, bytessize, 0x0, outString, strlen);
}
