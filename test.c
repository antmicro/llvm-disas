//
// Copyright (c) 2019-2021 Antmicro
//
// This file is licensed under the Apache License 2.0.
// Full license text is available in the 'LICENSE' file.
//
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "llvm-disas.h"

uint64_t strtohex64(const char *str, unsigned int *size)
{
    errno = 0;
    char *endptr;

    uint64_t hex = (uint64_t) strtoull(str, &endptr, 16);

    if (errno != 0) {
        perror("strtohex64");
        exit(EXIT_FAILURE);
    } else if (endptr == str) {
        fprintf(stderr, "strtohex64: '%s' is not a valid HEX number!\n", str);
        exit(EXIT_FAILURE);
    }

    unsigned int len = (unsigned int) (endptr - str);
    if (len % 2 != 0) {
        len++;
    }

    *size = len / 2;

    return hex;
}

int main(int argc, char **argv)
{
    if (argc != 4) {
        fprintf(stderr, "Usage: %s {cpu-arch} {cpu-model} {block}\n", argv[0]);
        fprintf(stderr, "  {block} must be a valid HEX (max 8B)\n");
        exit(EXIT_FAILURE);
    }

    unsigned int size = 0;
    uint64_t bytes = strtohex64(argv[3], &size);

    void *dc = llvm_create_disasm_cpu(argv[1], argv[2]);
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

    return 0;
}
