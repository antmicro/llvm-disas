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

// Only used by the llvm_create_disasm_cpu; not in llvm-disas.h
static void *create_riscv_cpu(const char *tripleName, const char *cpu)
{
    // CPU name must begin with either "rv32" or "rv64" to pass features
    const char *model = NULL;
    if (strncasecmp(cpu, "rv32", 4) == 0) {
        model = "generic-rv32";
    } else if (strncasecmp(cpu, "rv64", 4) == 0) {
        model = "generic-rv64";
    } else {
        return NULL;
    }

    // Seem much more than enough but added checks nevertheless, better safe than sorry.
    #define FEATURE_MAX_CHARS 100
    #define FEATURES_MAX_CHARS 2000

    char features[FEATURES_MAX_CHARS] = {0};
    char *next_c = (char *)(cpu + 4);
    while (*next_c != '\0') {
        char feature[FEATURE_MAX_CHARS] = {0};
        if (*next_c == '_') {  // long extension like `zba` in `rv32i_zba`
            next_c++;
            for (int i = 0; i < FEATURE_MAX_CHARS && *next_c != '_' && *next_c != '\0'; i++, next_c++) {
                feature[i] = *next_c;
            }
            // Feature name was truncated.
            if (*next_c != '_' && *next_c != '\0') {
                return NULL;
            }
        } else {
            feature[0] = *next_c;
            next_c++;
        }

        size_t feature_length = strlen(feature);
        if (feature_length > 0) {
            // Base integer instruction set, Zicntr, Zicsr, Zifencei, Zihpm are implicitly enabled.
            // See: https://llvm.org/docs/RISCVUsage.html#riscv-i2p1-note
            if (strcasecmp(feature, "i") == 0
                    || strcasecmp(feature, "zicntr") == 0
                    || strcasecmp(feature, "zicsr") == 0
                    || strcasecmp(feature, "zifencei") == 0
                    || strcasecmp(feature, "zihpm") == 0) {
                continue;
            } else if (strcasecmp(feature, "g") == 0) {
                // `g` combines these features; leading `+` and trailing `,` are added in snprintf.
                strcpy(feature, "m,+a,+f,+d");
                feature_length = strlen(feature);
            }

            size_t current_length = strlen(features);
            // +2 for `+` before, `,` after. Trailing comma is accepted.
            if(current_length + feature_length + 2 >= FEATURES_MAX_CHARS - 1) {
                return NULL;
            }

            snprintf(&features[current_length], feature_length + 3, "+%s,", feature);
        }
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
