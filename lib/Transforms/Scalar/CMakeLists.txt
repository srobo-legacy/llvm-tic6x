add_llvm_library(LLVMScalarOpts
  ABCD.cpp
  ADCE.cpp
  AlignmentFixing.cpp
  BasicBlockPlacement.cpp
  CodeGenPrepare.cpp
  ConstantProp.cpp
  DCE.cpp
  DeadStoreElimination.cpp
  GEPSplitter.cpp
  GVN.cpp
  IndVarSimplify.cpp
  JumpThreading.cpp
  LICM.cpp
  LoopDeletion.cpp
  LoopIndexSplit.cpp
  LoopRotation.cpp
  LoopStrengthReduce.cpp
  LoopUnrollPass.cpp
  LoopUnswitch.cpp
  MemCpyOptimizer.cpp
  Reassociate.cpp
  Reg2Mem.cpp
  SCCP.cpp
  SCCVN.cpp
  Scalar.cpp
  ScalarReplAggregates.cpp
  SimplifyCFGPass.cpp
  SimplifyHalfPowrLibCalls.cpp
  SimplifyLibCalls.cpp
  TailDuplication.cpp
  TailRecursionElimination.cpp
  )

target_link_libraries (LLVMScalarOpts LLVMTransformUtils)
