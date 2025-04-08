//
// Copyright (c) 2019-2021 Antmicro
//
// This file is licensed under the Apache License 2.0.
// Full license text is available in the 'LICENSE' file.
//
#include "llvm-disas.h"

int LLVMDisasmInstruction(void *dc, uint8_t *bytes, uint64_t bytessize, uint64_t pc, char *outString, uint32_t strlen);
int LLVMSetDisasmCondexecState(void *dc, uint64_t state);

int llvm_disasm_instruction(void *dc, uint8_t *bytes, uint64_t bytessize, char *outString, uint32_t strlen)
{

    return LLVMDisasmInstruction(dc, bytes, bytessize, 0x0, outString, strlen);
}

int llvm_disasm_set_condexec_state(void *dc, uint64_t state)
{
    return LLVMSetDisasmCondexecState(dc, state);
}
