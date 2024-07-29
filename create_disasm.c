//
// Copyright (c) 2019-2024 Antmicro
//
// This file is licensed under the Apache License 2.0.
// Full license text is available in the 'LICENSE' file.
//
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "llvm-disas.h"

void *LLVMCreateDisasmCPU(const char *tripleName, const char *cpu, void *disInfo, int tagType, void *getOpInfo, void *symbolLookUp);

void *LLVMCreateDisasmCPUFeatures(const char *tripleName, const char *cpu, const char *features, void *disInfo, int tagType, void *getOpInfo, void *symbolLookUp);

int LLVMSetDisasmOptions(void *dc, uint64_t options);

#define LLVMDisassembler_Option_AsmPrinterVariant 4

// Allow even more than possible unique features in case of some duplicates.
#define MAX_FEATURES 100

// Only used by the llvm_create_disasm_cpu; not in llvm-disas.h
static void *create_riscv_cpu(const char *tripleName, const char *cpu)
{
    // CPU name must begin with either "rv32" or "rv64" to pass features
    const char *model = NULL;
    if (strncmp(cpu, "rv32", 4) == 0) {
        model = "generic-rv32";
    } else if (strncmp(cpu, "rv64", 4) == 0) {
        model = "generic-rv64";
    } else {
        return NULL;
    }

    char features[MAX_FEATURES * 3 + 1] = {0};
    char *next_c = (char *)(cpu + 4);
    int features_n = 0;
    while (*next_c != '\0' && features_n < MAX_FEATURES) {
        if (*next_c == 'g' && features_n <= MAX_FEATURES - 4) {
            // "g" combines "m", "a", "f" and "d" features
            strcat(features, "+m,+a,+f,+d,");
            features_n += 4;
        } else if (*next_c != 'i') { // Base integer instruction set is implicitly enabled
            char feature[4];
            // Invalid features, duplicates and extra commas are ignored gracefully.
            snprintf(feature, 4, "+%c,", *next_c);
            strcat(features, feature);
            features_n++;
        }
        next_c++;
    }

    return LLVMCreateDisasmCPUFeatures(tripleName, model, features, NULL, 0, NULL, NULL);
}

void *llvm_create_disasm_cpu_with_flags(const char *tripleName, const char *cpu, uint32_t flags)
{
    // Initializing LLVM architectures multiple times doesn't seem to cause any problems
    init_llvm_architecture(tripleName);

    if (strncmp(tripleName, "riscv32", 7) == 0 || strncmp(tripleName, "riscv64", 7) == 0) {
        void *ret_val = create_riscv_cpu(tripleName, cpu);
        if (ret_val != NULL) {
            return ret_val;
        }
    }

    void *dc = LLVMCreateDisasmCPU(tripleName, cpu, NULL, 0, NULL, NULL);
    if (!dc) {
        return NULL;
    }

    if (flags & ASM_ALTERNATE_DIALECT) {
        LLVMSetDisasmOptions(dc, LLVMDisassembler_Option_AsmPrinterVariant);
    }

    return dc;
}

void *llvm_create_disasm_cpu(const char *tripleName, const char *cpu)
{
    return llvm_create_disasm_cpu_with_flags(tripleName, cpu, 0);
}
