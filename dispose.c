//
// Copyright (c) 2019-2021 Antmicro
//
// This file is licensed under the Apache License 2.0.
// Full license text is available in the 'LICENSE' file.
//
#include "llvm-disas.h"

void LLVMDisasmDispose(void *dc);

void llvm_disasm_dispose(void *dc)
{

    LLVMDisasmDispose(dc);
}
