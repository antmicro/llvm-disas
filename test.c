//
// Copyright (c) 2019-2024 Antmicro
//
// This file is licensed under the Apache License 2.0.
// Full license text is available in the 'LICENSE' file.
//
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "llvm-disas.h"

int unhexnibble(char hex)
{
    hex = tolower(hex);
    if (hex >= '0' && hex <= '9') {
        return hex - '0';
    } else if (hex >= 'a' && hex <= 'f') {
        return hex - 'a' + 0xa;
    } else {
        return -1;
    }
}

ssize_t unhexdump(const char *str, uint8_t *out, size_t max_out_size)
{
    ssize_t nib_idx = 0;

    memset(out, 0, max_out_size);

    while (*str && nib_idx / 2 < max_out_size) {
        if (isspace(*str)) {
            ++str;
            continue;
        }

        int nib = unhexnibble(*str);
        if (nib < 0) {
            return -1;
        }

        // We number the nibbles from left to right as in a displayed number, so high nibbles are at even indices
        bool high_nibble = nib_idx % 2 == 0;
        out[nib_idx / 2] |= nib << (high_nibble ? 4 : 0);
        ++nib_idx;
        ++str;
    }

    return (nib_idx + 1) / 2;
}

char hexnibble(uint8_t nibble)
{
    if (nibble >= 0x0 && nibble <= 0x9) {
        return '0' + nibble;
    } else {
        return 'a' + nibble - 0xa;
    }
}

char *hexdump(const uint8_t *data, size_t size)
{
    char *out = calloc(size + 1, 3);
    if (!out) {
        return NULL;
    }

    for (size_t i = 0; i < size; ++i) {
        out[i * 3 + 0] = hexnibble(data[i] >> 4);
        out[i * 3 + 1] = hexnibble(data[i] & 0xf);
        out[i * 3 + 2] = ' ';
    }

    return out;
}

void do_assemble(int argc, char **argv, uint32_t flags, uint64_t address)
{
    uint8_t *out;
    size_t out_len;

    init_llvm_architecture(argv[0]);
    if (!llvm_asm(argv[0], argv[1], flags, argv[2], address, &out, &out_len)) {
        // If assembly failed, the output contains the error message instead of assembled instruction data
        fprintf(stderr, "Failed to assemble. Reason: %s\n", out);
        exit(EXIT_FAILURE);
    }

    char *hex = hexdump(out, out_len);
    printf("%s\n", hex ? hex : "Failed to allocate memory for hex dump!");

    free(hex);
    llvm_free_asm_result(out);
}

void do_disassemble(int argc, char **argv, uint32_t flags)
{
    uint8_t bytes[1024];
    ssize_t size = unhexdump(argv[2], bytes, sizeof(bytes));
    if (size < 0) {
        fprintf(stderr, "'%s' is not a valid hex string\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    void *dc = llvm_create_disasm_cpu_with_flags(argv[0], argv[1], flags);
    if (dc == NULL)
    {
        fprintf(stderr, "Error creating context (invalid {cpu-arch}?).\n");
        exit(EXIT_FAILURE);
    } 

    unsigned int buf_size = 1024;
    char *buf = (char *) malloc(buf_size);

    unsigned int cur_size = size;
    uint8_t *cur_bytes = (uint8_t *) &bytes;

    while (cur_size != 0) {
        memset(buf, 0, buf_size);

        int cnt = llvm_disasm_instruction(dc, cur_bytes, cur_size, buf, buf_size);

        if (cnt > 0) {
            printf("%s\n", buf);
        } else {
            break;
        }

        cur_bytes += cnt;
        cur_size -= cnt;
    }

    free(buf);
    llvm_disasm_dispose(dc);
}

int main(int argc, char **argv)
{
    int opt;
    int assemble = 0;
    uint32_t flags = 0;
    uint64_t address = 0;

    while ((opt = getopt(argc, argv, "ab:d")) != -1) {
        switch (opt) {
        case 'a':
            assemble = 1;
            break;
        case 'b': {
            char *end;
            address = strtoull(optarg, &end, 0);
            if (*optarg == '\0' || *end != '\0') {
                fprintf(stderr, "Invalid base address %s\n", optarg);
                exit(EXIT_FAILURE);
            }
            break;
        }
        case 'd':
            flags |= ASM_ALTERNATE_DIALECT;
            break;
        }
    }

    char *program = argv[0];
    argc -= optind;
    argv += optind;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s [-ad] [-b base] {cpu-arch} {cpu-model} {block}\n", program);
        fprintf(stderr, "  {block} must be a valid HEX string when disassembling\n");
        fprintf(stderr, "  {block} must be valid assembly code when assembling\n");
        fprintf(stderr, "  -a: assemble (default: disassemble)\n");
        fprintf(stderr, "  -b: code base address for assembly\n");
        fprintf(stderr, "  -d: use alternate assembly dialect (for x86: Intel syntax)\n");
        exit(EXIT_FAILURE);
    }

    if (assemble) {
        do_assemble(argc, argv, flags, address);
    } else {
        do_disassemble(argc, argv, flags);
    }

    return 0;
}
