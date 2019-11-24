#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define EXTERNAL extern "C"

#define ADD_ARCH(x) EXTERNAL void LLVMInitialize ## x ## Disassembler(); \
                    EXTERNAL void LLVMInitialize ## x ## TargetInfo(); \
		    EXTERNAL void LLVMInitialize ## x ## TargetMC();

#define INIT_ARCH(x) LLVMInitialize ## x ## Disassembler(); \
                     LLVMInitialize ## x ## TargetInfo(); \
                     LLVMInitialize ## x ## TargetMC();


ADD_ARCH(ARM);
ADD_ARCH(RISCV);
ADD_ARCH(X86);
ADD_ARCH(AArch64);
ADD_ARCH(Sparc);
ADD_ARCH(PowerPC);
ADD_ARCH(Mips);

EXTERNAL int LLVMDisasmInstruction(void *dc, uint8_t *bytes, uint64_t bytessize, uint64_t pc, char * outString, uint32_t strlen);
typedef int(* LLVMOpInfoCallback) (void *DisInfo, uint64_t PC, uint64_t Offset, uint64_t Size, int TagType, void *TagBuf);
typedef const char*(* LLVMSymbolLookupCallback) (void *DisInfo, uint64_t ReferenceValue, uint64_t *ReferenceType, uint64_t ReferencePC, const char **ReferenceName);

EXTERNAL void* LLVMCreateDisasmCPUFeatures(const char *tripleName, const char* cpu, const char *features, void* disInfo, int tagType, LLVMOpInfoCallback getOpInfo, LLVMSymbolLookupCallback symbolLookUp);

#define STUB { printf("WARNING: stub %s\n", __func__); }

// 
// TODO: those functions are just stubs
// of zlib and curses functions.
//

EXTERNAL void setupterm() STUB;
EXTERNAL void tigetnum() STUB;
EXTERNAL void set_curterm() STUB;
EXTERNAL void del_curterm() STUB;
EXTERNAL void uncompress() STUB;
EXTERNAL void crc32() STUB;
EXTERNAL void compress2() STUB;
EXTERNAL void compressBound() STUB;


int opinfoCallback(void *DisInfo, uint64_t pc, uint64_t offset, uint64_t size, int tagType, void *tagBuf) {
	printf("here %s\n", __func__);
	return 1;
}

const char *symbolLookupCallback(void *DisInfo, uint64_t ReferenceValue, uint64_t *ReferenceType, uint64_t ReferencePC, const char **ReferenceName) {
        printf("here %s\n", __func__);
	return "unknown";
}

int main()
{
	INIT_ARCH(ARM);
	INIT_ARCH(RISCV);
	INIT_ARCH(X86);
	INIT_ARCH(AArch64);
	INIT_ARCH(Sparc);
	INIT_ARCH(PowerPC);
	INIT_ARCH(Mips);


/////////////////////////////////////////////////////////////////////////
// TODO: test
void *dc = LLVMCreateDisasmCPUFeatures("riscv32", "generic-rv32", "+a", NULL, 0, opinfoCallback, symbolLookupCallback);
printf("dc = %p\n", dc);

char *st = (char*)malloc(1024);
int sz = 8;
uint32_t *bytes = (uint32_t*)malloc(sz);

bytes[0] = 0x00020493;
bytes[1] = 0x40848433;



uint64_t pc = 0x0;

int cnt = LLVMDisasmInstruction(dc, (uint8_t*)bytes, sz, pc, st, 1024);
printf("cnt = %d\n", cnt);
if (cnt > 0)
printf("%s\n", st);
return 0;
}
