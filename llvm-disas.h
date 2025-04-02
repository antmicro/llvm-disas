//
// Copyright (c) 2019-2024 Antmicro
//
// This file is licensed under the Apache License 2.0.
// Full license text is available in the 'LICENSE' file.
//
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Flags for llvm_create_disasm_cpu_with_flags and llvm_asm
enum {
    ASM_ALTERNATE_DIALECT = (1 << 0),
};

void llvm_disasm_AArch64_init(void);

void llvm_disasm_ARM_init(void);

void llvm_disasm_Mips_init(void);

void llvm_disasm_PowerPC_init(void);

void llvm_disasm_RISCV_init(void);

void llvm_disasm_Sparc_init(void);

void llvm_disasm_X86_init(void);

void llvm_disasm_MSP430_init(void);

void init_llvm_architecture(const char *arch);

void *llvm_create_disasm_cpu_with_flags(const char *tripleName, const char *cpu, uint32_t flags);

__attribute__((deprecated("Alias to preserve ABI compatibility. Use llvm_create_disasm_cpu_with_flags instead.")))
void *llvm_create_disasm_cpu(const char *tripleName, const char *cpu);

int llvm_disasm_instruction(void *dc, uint8_t *bytes, uint64_t bytessize, char *outString, uint32_t strlen);

int llvm_disasm_set_condexec_state(void *dc, uint64_t state);

void llvm_disasm_dispose(void *dc);

bool llvm_asm(const char *arch, const char *cpu, uint32_t flags, const char *instructions, uint64_t addr, uint8_t **out, size_t *outLen);

void llvm_free_asm_result(void *result);

#ifdef __cplusplus
}
#endif
