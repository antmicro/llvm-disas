//
// Copyright (c) 2019-2021 Antmicro
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

// Allow even more than possible unique features in case of some duplicates.
#define MAX_FEATURES 100

// Only used by the llvm_create_disasm_cpu; not in llvm-disas.h
void *create_riscv_cpu(const char *tripleName, const char *cpu)
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

// Architecture aliases based on LLVM's 'parseArch' defined in lib/support/Triple.cpp
void init_llvm_architecture(const char *arch){
    if ((strncmp(arch, "arm", 3) == 0 && strncmp(arch, "arm64", 5) != 0)
                || strncmp(arch, "thumb", 5) == 0) {
        llvm_disasm_ARM_init();
    } else if (strncmp(arch, "aarch64", 7) == 0 || strncmp(arch, "arm64", 5) == 0) {
        llvm_disasm_AArch64_init();
    } else if (strncmp(arch, "mips", 4) == 0) {
        llvm_disasm_Mips_init();
    } else if (strncmp(arch, "powerpc", 7) == 0 || strncmp(arch, "ppc", 3) == 0) {
        llvm_disasm_PowerPC_init();
    } else if (strncmp(arch, "riscv", 5) == 0) {
        llvm_disasm_RISCV_init();
    } else if (strncmp(arch, "sparc", 5) == 0) {
        llvm_disasm_Sparc_init();
    } else if (strncmp(arch, "x86", 3) == 0 || strcmp(arch, "amd64") == 0
                // All i?86 from i386 to i986 are valid LLVM architectures
                || (strncmp(arch, "i", 1) == 0 && strncmp(arch+2, "86", 2) == 0)) {
        llvm_disasm_X86_init();
    }
}

void *llvm_create_disasm_cpu(const char *tripleName, const char *cpu)
{
    // Initializing LLVM architectures multiple times doesn't seem to cause any problems
    init_llvm_architecture(tripleName);

    if (strncmp(tripleName, "riscv32", 7) == 0 || strncmp(tripleName, "riscv64", 7) == 0) {
        void *ret_val = create_riscv_cpu(tripleName, cpu);
        if (ret_val != NULL) {
            return ret_val;
        }
    }

    return LLVMCreateDisasmCPU(tripleName, cpu, NULL, 0, NULL, NULL);
}
