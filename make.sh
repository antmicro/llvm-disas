if [ ! -e llvm-libs ] ; then
        mkdir llvm-libs 2> /dev/null
        cd llvm-libs
        wget -c http://releases.llvm.org/9.0.0/clang+llvm-9.0.0-x86_64-pc-linux-gnu.tar.xz -O - | tar x --xz --wildcards --strip-components=2 "clang+llvm-9.0.0-x86_64-pc-linux-gnu/lib/libLLVM*.a"
        cd ..
fi

# LLVM_LIBS="-Lllvm-libs/ llvm-libs/libLLVM*Disassembler.a llvm-libs/libLLVM*AsmParser.a llvm-libs/libLLVM*CodeGen.a llvm-libs/libLLVM*Desc.a llvm-libs/libLLVM*Info.a llvm-libs/libLLVM*Utils.a -lLLVMAggressiveInstCombine -lLLVMAnalysis -lLLVMBinaryFormat -lLLVMBitReader -lLLVMBitstreamReader -lLLVMBitWriter -lLLVMCodeGen -lLLVMCore -lLLVMCoroutines -lLLVMCoverage -lLLVMDebugInfoCodeView -lLLVMDebugInfoDWARF -lLLVMDebugInfoGSYM -lLLVMDebugInfoMSF -lLLVMDebugInfoPDB -lLLVMDemangle -lLLVMDlltoolDriver -lLLVMExecutionEngine -lLLVMFuzzMutate -lLLVMGlobalISel -lLLVMInstCombine -lLLVMInstrumentation -lLLVMInterpreter -lLLVMipo -lLLVMIRReader -lLLVMJITLink -lLLVMLibDriver -lLLVMLineEditor -lLLVMLinker -lLLVMLTO -lLLVMMCA -lLLVMMCDisassembler -lLLVMMCJIT -lLLVMMCParser -lLLVMMC -lLLVMMIRParser -lLLVMObjCARCOpts -lLLVMObject -lLLVMObjectYAML -lLLVMOption -lLLVMOrcJIT -lLLVMPasses -lLLVMProfileData -lLLVMRemarks -lLLVMRuntimeDyld -lLLVMScalarOpts -lLLVMSelectionDAG -lLLVMSupport -lLLVMSymbolize -lLLVMTableGen -lLLVMTarget -lLLVMTextAPI -lLLVMTransformUtils -lLLVMVectorize -lLLVMWindowsManifest -lLLVMXRay -lLLVMMC -lLLVMBinaryFormat"

g++ main.cpp -Wl,-Bstatic llvm-libs/libLLVM{*,BinaryFormat,MC}.a stub.cpp -Wl,-Bdynamic -static-libstdc++ -static-libgcc -pthread


