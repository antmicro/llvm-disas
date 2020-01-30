#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "llvm.h"

uint64_t strtohex64(const char* str){
	errno = 0;
	char* endptr;

	uint64_t hex = (uint64_t) strtoull(str, &endptr, 16);

	if (errno != 0){
		perror("strtohex64");
		exit(EXIT_FAILURE);
	} else if (endptr == str){
		fprintf(stderr, "strtohex64: '%s' is not a valid HEX number!\n", str);
		exit(EXIT_FAILURE);
	}

	return hex;
}

void llvm_init_all(){
	llvm_disasm_AArch64_init();
	llvm_disasm_ARM_init();
	llvm_disasm_Mips_init();
	llvm_disasm_PowerPC_init();
	llvm_disasm_RISCV_init();
	llvm_disasm_Sparc_init();
	llvm_disasm_X86_init();
}

int main(int argc, char** argv)
{
	if (argc != 4 && argc != 5){
		fprintf(stderr, "Usage: %s tripleName CPU bytes [addr_start = 0x0]\n", argv[0]);
		fprintf(stderr, "  (bytes and addr_start should be HEX – 64-bit max)\n");
		exit(EXIT_FAILURE);
	}

	uint64_t addr_start = 0x0;
	if (argc == 5){
		addr_start = strtohex64(argv[4]);
	}

	uint64_t bytes = strtohex64(argv[3]);

	llvm_init_all();

	//TODO: Check if tripleName and CPU are valid
	void *dc = llvm_create_disasm_cpu(argv[1], argv[2]);

	unsigned int buf_size = 1024;
	char *buf = (char*) malloc(buf_size);

	unsigned int cur_size = sizeof(bytes);
	uint8_t* cur_bytes = (uint8_t*) &bytes;

	while(cur_size != 0){
		memset(buf, 0, buf_size);

		int cnt = llvm_disasm_instruction(dc, cur_bytes, cur_size, addr_start, buf, buf_size);

		if (cnt > 0){
			printf("%s\n", buf);
		} else {
			break;
		}

		cur_bytes += cnt;
		addr_start += cnt;
		cur_size -= cnt;
	}

	free(buf);
	llvm_disasm_dispose(dc);

	return 0;
}
