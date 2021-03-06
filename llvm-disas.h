//
// Copyright (c) 2019-2021 Antmicro
//
// This file is licensed under the Apache License 2.0.
// Full license text is available in the 'LICENSE' file.
//
#include <stdint.h>

void llvm_disasm_AArch64_init(void);

void llvm_disasm_ARM_init(void);

void llvm_disasm_Mips_init(void);

void llvm_disasm_PowerPC_init(void);

void llvm_disasm_RISCV_init(void);

void llvm_disasm_Sparc_init(void);

void llvm_disasm_X86_init(void);

void *llvm_create_disasm_cpu(const char *tripleName, const char *cpu);

int llvm_disasm_instruction(void *dc, uint8_t *bytes, uint64_t bytessize, char *outString, uint32_t strlen);

void llvm_disasm_dispose(void *dc);
