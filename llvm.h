#include <stdint.h>

typedef int(* LLVMOpInfoCallback) (void *DisInfo, uint64_t PC, uint64_t Offset, uint64_t Size, int TagType, void *TagBuf);
typedef const char*(* LLVMSymbolLookupCallback) (void *DisInfo, uint64_t ReferenceValue, uint64_t *ReferenceType, uint64_t ReferencePC, const char **ReferenceName);

void llvm_disasm_AArch64_init(void);

void llvm_disasm_ARM_init(void);

void llvm_disasm_Mips_init(void);

void llvm_disasm_PowerPC_init(void);

void llvm_disasm_RISCV_init(void);

void llvm_disasm_Sparc_init(void);

void llvm_disasm_X86_init(void);

void* llvm_create_disasm_cpu_features(const char *tripleName, const char* cpu, const char *features, void* disInfo, int tagType, LLVMOpInfoCallback getOpInfo, LLVMSymbolLookupCallback symbolLookUp);

void* llvm_create_disasm_cpu(const char *tripleName, const char* cpu, void* disInfo, int tagType, LLVMOpInfoCallback getOpInfo, LLVMSymbolLookupCallback symbolLookUp);

int llvm_disasm_instruction(void *dc, uint8_t *bytes, uint64_t bytessize, uint64_t pc, char * outString, uint32_t strlen);

void llvm_disasm_dispose(void *dc);
