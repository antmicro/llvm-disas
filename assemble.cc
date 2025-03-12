//
// Copyright (c) 2019-2024 Antmicro
//
// This file is licensed under the Apache License 2.0.
// Full license text is available in the 'LICENSE' file.
//
#include <algorithm>
#include <cstring>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCValue.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/TargetParser/RISCVISAInfo.h"
#include "llvm/TargetParser/Triple.h"
#include "llvm/Support/SourceMgr.h"

#include "llvm-disas.h"

using namespace llvm;

class MemoryObjectWriter : public MCObjectWriter
{
public:
    MemoryObjectWriter(raw_ostream &OS, bool &Failed, std::vector<SMDiagnostic> &Diagnostics,
                       uint64_t Address)
        : OS(OS), Failed(Failed), Diagnostics(Diagnostics), Address(Address)
    {
    }

    virtual void executePostLayoutBinding(MCAssembler &Asm)
    {
    }

    void recordRelocation(MCAssembler &Asm, const MCFragment *Fragment,
                          const MCFixup &Fixup, MCValue Target,
                          uint64_t &FixedValue)
    {
        // If we see an undefined symbol, consider assembly failed.
        // All referenced labels must be defined within the provided code block.
        checkSymbolDefined(Target.getSymA());
        checkSymbolDefined(Target.getSymB());

        // Simulate the target section starting at Address.
        FixedValue += Address;
    }

    uint64_t writeObject(MCAssembler &Asm)
    {
        for (const MCSection &Section : Asm) {
            if (Section.getName() == ".text") {
                Asm.writeSectionData(OS, &Section);
            }
        }
        return 0;
    }

private:
    void checkSymbolDefined(const MCSymbolRefExpr *SymRef)
    {
        if (!SymRef) {
            return;
        }

        const auto &Symbol = SymRef->getSymbol();
        if (!Symbol.isDefined()) {
            std::string Message = "Undefined label ";
            Message += Symbol.getName();
            Diagnostics.emplace_back("", SourceMgr::DiagKind::DK_Error, Message);
            Failed = true;
        }
    }

    raw_ostream &OS;
    bool &Failed;
    std::vector<SMDiagnostic> &Diagnostics;
    uint64_t Address;
};

static void SetError(SmallVector<char> &Out, std::string const &Error)
{
    Out.clear();
    Out.reserve(Error.length() + 1);
    std::copy(Error.cbegin(), Error.cend(), std::back_inserter(Out));
    Out.push_back('\0');
}

static bool AssembleInner(StringRef &Arch, StringRef &CPU, uint32_t Flags,
                                   StringRef &InputStr, uint64_t Address,
                                   SmallVector<char> &OutBytes)
{
    std::string TargetLookupError;
    const Target *TheTarget = TargetRegistry::lookupTarget(std::string(Arch), TargetLookupError);

    if (!TheTarget) {
        SetError(OutBytes, "Unknown target: " + TargetLookupError);
        return false;
    }

    std::vector<SMDiagnostic> Diagnostics;
    auto DiagHandler = [](const SMDiagnostic &Diag, void *Context) {
        static_cast<std::vector<SMDiagnostic> *>(Context)->push_back(Diag);
    };

    SourceMgr SrcMgr;
    SrcMgr.setDiagHandler(DiagHandler, &Diagnostics);

    std::unique_ptr<MemoryBuffer> BufferPtr = MemoryBuffer::getMemBuffer(InputStr);
    SrcMgr.AddNewSourceBuffer(std::move(BufferPtr), SMLoc());

    std::unique_ptr<MCRegisterInfo> MRI(TheTarget->createMCRegInfo(Arch));
    if (!MRI) {
        SetError(OutBytes, "Unable to create target register info");
        return false;
    }

    std::unique_ptr<MCAsmInfo> MAI(TheTarget->createMCAsmInfo(*MRI, Arch, MCTargetOptions()));
    if (!MAI) {
        SetError(OutBytes, "Unable to create target asm info");
        return false;
    }

    Triple TheTriple(Arch);
    std::string MCPU = CPU.str();
    // For RISC-V the directly supported CPU names are the 2 generic cores below, which have no features, and
    // real cores such as `sifive-u54` with their corresponding features.
    // See https://github.com/llvm/llvm-project/blob/329fda39c507e8740978d10458451dcdb21563be/llvm/lib/Target/RISCV/RISCV.td#L442
    // We additionally allow passing a CPU string like `rv32imac`, in this case we take the generic core and
    // apply the features specified in the CPU string.
    if (TheTriple.isRISCV()) {
        if (TheTriple.isArch64Bit()) {
            MCPU = "generic-rv64";
        } else {
            MCPU = "generic-rv32";
        }
    }

    std::unique_ptr<MCSubtargetInfo> STI(TheTarget->createMCSubtargetInfo(Arch, MCPU, ""));
    if (!STI->isCPUStringValid(MCPU)) {
        SetError(OutBytes, "Invalid CPU model");
        return false;
    }

    // Apply RISC-V features
    if (TheTriple.isRISCV() && CPU.starts_with("rv")) {
        auto IsaInfo = RISCVISAInfo::parseArchString(CPU, false);
        if (auto E = IsaInfo.takeError()) {
            std::string ErrorStr = "Invalid RISC-V feature string: ";
            ErrorStr += toString(std::move(E));
            SetError(OutBytes, ErrorStr);
            return false;
        }

        for (const std::string &Feature : (*IsaInfo)->toFeatures()) {
            STI->ApplyFeatureFlag(Feature);
        }
    }

    MCContext Ctx(TheTriple, MAI.get(), MRI.get(), STI.get(), &SrcMgr);
    std::unique_ptr<MCObjectFileInfo> MOFI(TheTarget->createMCObjectFileInfo(Ctx, /*PIC*/ false));
    Ctx.setObjectFileInfo(MOFI.get());

    MCTargetOptions MCOptions;
    std::unique_ptr<MCInstrInfo> MCII(TheTarget->createMCInstrInfo());
    std::unique_ptr<MCCodeEmitter> CE(TheTarget->createMCCodeEmitter(*MCII, Ctx));
    std::unique_ptr<MCAsmBackend> MAB(TheTarget->createMCAsmBackend(*STI, *MRI, MCOptions));

    raw_svector_ostream VOS(OutBytes);
    bool WriterFailed = false;
    std::unique_ptr<MCObjectWriter> OW =
        std::make_unique<MemoryObjectWriter>(VOS, WriterFailed, Diagnostics, Address);

    std::unique_ptr<MCStreamer> Str(TheTarget->createMCObjectStreamer(
	    TheTriple, Ctx, std::move(MAB), std::move(OW), std::move(CE), *STI));

    std::unique_ptr<MCAsmParser> Parser(createMCAsmParser(SrcMgr, Ctx, *Str, *MAI));

    Parser->setAssemblerDialect((Flags & ASM_ALTERNATE_DIALECT) ? 1u : 0u);

    std::unique_ptr<MCTargetAsmParser> TAP(
        TheTarget->createMCAsmParser(*STI, *Parser, *MCII, MCOptions));
    if (!TAP) {
        SetError(OutBytes, "Failed to create target assembly parser");
        return false;
    }
    Parser->setTargetParser(*TAP);

    bool HadError = Parser->Run(/*NoInitialTextSection*/ false);
    if (HadError || WriterFailed) {
        if (Diagnostics.empty()) {
            SetError(OutBytes, "Unknown error");
            return false;
        }

        std::string DiagString;
        raw_string_ostream DiagOs(DiagString);

        for (const SMDiagnostic &diagnostic : Diagnostics) {
            diagnostic.print(nullptr, DiagOs);
            DiagOs << "\n";
        }

        SetError(OutBytes, DiagString);
        return false;
    }

    return true;
}

// API
extern "C" {
bool llvm_asm(const char *arch, const char *cpu, uint32_t flags, const char *instructions,
              uint64_t addr, uint8_t **out, size_t *outLen)
{
    StringRef Arch(arch);
    StringRef CPU(cpu);
    StringRef Instructions(instructions);
    SmallVector<char> OutVec;

    // If assembly fails, the output byte vector will contain the error message instead of
    // assembled instruction data.
    bool Ok = AssembleInner(Arch, CPU, flags, Instructions, addr, OutVec);

    // Copy the output to a region allocated with malloc so that we can free it by passing
    // the same pointer to free
    *out = static_cast<uint8_t *>(malloc(OutVec.size()));
    if(*out == nullptr) {
        // If allocation fails, force the length to 0
        *outLen = 0;
        return false;
    }

    memcpy(*out, OutVec.data(), OutVec.size());
    *outLen = OutVec.size();
    return Ok;
}

void llvm_free_asm_result(void *result)
{
    free(result);
}
}
