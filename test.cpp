#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "llvm-init.hpp"

int main()
{
	llvm_disas_init();

/////////////////////////////////////////////////////////////////////////
// TODO: test
void *dc = llvm_create_disasm_cpu_features("riscv32", "generic-rv32", "+a", NULL, 0, opinfoCallback, symbolLookupCallback);
printf("dc = %p\n", dc);

char *st = (char*)malloc(1024);
int sz = 8;
uint32_t *bytes = (uint32_t*)malloc(sz);

bytes[0] = 0x00020493;
bytes[1] = 0x40848433;



uint64_t pc = 0x0;

int cnt = llvm_disasm_instruction(dc, (uint8_t*)bytes, sz, pc, st, 1024);
printf("cnt = %d\n", cnt);
if (cnt > 0)
printf("%s\n", st);

llvm_disasm_dispose(dc);
return 0;
}
