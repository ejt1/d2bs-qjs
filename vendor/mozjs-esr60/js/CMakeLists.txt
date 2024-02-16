add_definitions(-DNOMINMAX)
add_definitions(-DPSAPI_VERSION=1)
add_definitions(-DWIN32_LEAN_AND_MEAN)
add_definitions(-D_USE_MATH_DEFINES)

if (MOZJS_THREADSAFE)
  set(JS_THREADSAFE ON)
endif()

set(JS_GC_ZEAL ON)

# Forced ON because wont compile without
set(ENABLE_TRACE_LOGGING ON)
set(MOZJS_ENABLE_ION ON)
add_definitions(-DJS_ION)

if (TARGET_CPU MATCHES "x86_64")
  set(JS_PUNBOX64 1)
  set(JS_CODEGEN_X64 ON)
  add_definitions(-DHAVE_64BIT_BUILD)
  add_definitions(-DJS_64BIT)
else()
  set(JS_NUNBOX32 1)
  set(JS_CODEGEN_X86 ON)
  add_definitions(-DJS_32BIT)
endif()

if (CMAKE_BUILD_TYPE MATCHES "Debug")
  set(JS_DEBUG ON)
endif()

#####################################################################
## SpiderMonkey (Library)
#####################################################################
# Listed in the same order as in js/src/moz.build
# Early version of js did not provide full path to source files
set(JS_SRCS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/intl/Collator.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/intl/CommonFunctions.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/intl/DateTimeFormat.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/intl/IntlObject.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/intl/NumberFormat.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/intl/PluralRules.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/intl/RelativeTimeFormat.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/intl/SharedIntlData.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/AtomicsObject.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/DataViewObject.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/Eval.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/JSON.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/MapObject.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/ModuleObject.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/Object.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/Profilers.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/Promise.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/Reflect.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/ReflectParse.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/RegExp.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/SIMD.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/Stream.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/String.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/Symbol.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/TestingFunctions.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/TypedObject.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/WeakMapObject.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/WeakSetObject.cpp"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/devtools/sharkctl.cpp"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/ds/Bitmap.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/ds/LifoAlloc.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/ds/MemoryProtectionExceptionHandler.cpp"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/frontend/BytecodeCompiler.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/frontend/BytecodeEmitter.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/frontend/FoldConstants.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/frontend/NameFunctions.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/frontend/ParseNode.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/frontend/Parser.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/frontend/TokenStream.cpp"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/gc/Allocator.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/gc/AtomMarking.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/gc/Barrier.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/gc/GC.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/gc/GCTrace.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/gc/Marking.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/gc/Memory.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/gc/Nursery.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/gc/PublicIterators.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/gc/RootMarking.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/gc/Statistics.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/gc/StoreBuffer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/gc/Tracer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/gc/Verifier.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/gc/WeakMap.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/gc/WeakMapPtr.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/gc/Zone.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/gc/ZoneGroup.cpp"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/irregexp/NativeRegExpMacroAssembler.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/irregexp/RegExpAST.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/irregexp/RegExpCharacters.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/irregexp/RegExpEngine.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/irregexp/RegExpInterpreter.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/irregexp/RegExpMacroAssembler.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/irregexp/RegExpParser.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/irregexp/RegExpStack.cpp"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/AliasAnalysis.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/AliasAnalysisShared.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/AlignmentMaskAnalysis.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/BacktrackingAllocator.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/Bailouts.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/BaselineBailouts.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/BaselineCacheIRCompiler.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/BaselineCompiler.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/BaselineDebugModeOSR.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/BaselineFrame.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/BaselineFrameInfo.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/BaselineIC.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/BaselineInspector.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/BaselineJIT.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/BitSet.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/BytecodeAnalysis.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/C1Spewer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/CacheIR.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/CacheIRCompiler.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/CacheIRSpewer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/CodeGenerator.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/CompileWrappers.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/Disassembler.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/EagerSimdUnbox.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/EdgeCaseAnalysis.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/EffectiveAddressAnalysis.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/ExecutableAllocator.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/FlowAliasAnalysis.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/FoldLinearArithConstants.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/InstructionReordering.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/Ion.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/IonAnalysis.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/IonBuilder.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/IonCacheIRCompiler.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/IonControlFlow.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/IonIC.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/IonOptimizationLevels.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/Jit.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/JitcodeMap.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/JitFrames.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/JitOptions.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/JitSpewer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/JSJitFrameIter.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/JSONSpewer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/LICM.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/Linker.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/LIR.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/LoopUnroller.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/Lowering.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/MacroAssembler.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/MCallOptimize.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/MIR.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/MIRGraph.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/MoveResolver.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/OptimizationTracking.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/PerfSpewer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/ProcessExecutableMemory.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/RangeAnalysis.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/Recover.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/RegisterAllocator.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/RematerializedFrame.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/Safepoints.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/ScalarReplacement.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/shared/Assembler-shared.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/shared/BaselineCompiler-shared.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/shared/CodeGenerator-shared.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/shared/Disassembler-shared.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/shared/Lowering-shared.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/SharedIC.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/Sink.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/Snapshots.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/StupidAllocator.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/TypedObjectPrediction.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/TypePolicy.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/ValueNumbering.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/VMFunctions.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/WasmBCE.cpp"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/perf/jsperf.cpp"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/proxy/BaseProxyHandler.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proxy/CrossCompartmentWrapper.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proxy/DeadObjectProxy.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proxy/OpaqueCrossCompartmentWrapper.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proxy/Proxy.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proxy/ScriptedProxyHandler.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proxy/SecurityWrapper.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proxy/Wrapper.cpp"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/threading/Mutex.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/threading/ProtectedData.cpp"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/util/AllocPolicy.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/util/DoubleToString.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/util/NativeStack.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/util/Printf.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/util/StringBuffer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/util/Text.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/util/Unicode.cpp"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/ArgumentsObject.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/ArrayBufferObject.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/AsyncFunction.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/AsyncIteration.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/BytecodeUtil.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/Caches.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/CallNonGenericMethod.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/CharacterEncoding.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/CodeCoverage.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/Compression.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/DateTime.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/Debugger.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/DebuggerMemory.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/EnvironmentObject.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/ErrorObject.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/ErrorReporting.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/ForOfIterator.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/GeckoProfiler.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/GeneratorObject.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/GlobalObject.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/HelperThreads.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/Id.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/Initialization.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/Interpreter.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/Iteration.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/JSAtom.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/JSCompartment.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/JSContext.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/JSFunction.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/JSObject.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/JSONParser.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/JSONPrinter.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/JSScript.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/MemoryMetrics.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/NativeObject.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/ObjectGroup.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/PIC.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/Printer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/Probes.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/ProxyObject.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/Realm.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/ReceiverGuard.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/RegExpObject.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/RegExpStatics.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/Runtime.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/SavedStacks.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/Scope.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/SelfHosting.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/Shape.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/SharedArrayObject.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/SharedImmutableStringsCache.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/Stack.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/Stopwatch.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/StringType.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/StructuredClone.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/SymbolType.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/TaggedProto.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/Time.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/TypedArrayObject.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/TypeInference.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/UbiNode.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/UbiNodeCensus.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/UbiNodeShortestPaths.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/UnboxedObject.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/Value.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/Xdr.cpp"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/AsmJS.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/WasmBaselineCompile.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/WasmBinaryIterator.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/WasmBinaryToAST.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/WasmBinaryToText.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/WasmBuiltins.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/WasmCode.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/WasmCompartment.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/WasmCompile.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/WasmDebug.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/WasmFrameIter.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/WasmGenerator.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/WasmInstance.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/WasmIonCompile.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/WasmJS.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/WasmModule.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/WasmProcess.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/WasmSignalHandlers.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/WasmStubs.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/WasmTable.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/WasmTextToBinary.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/WasmTextUtils.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/WasmTypes.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/WasmValidate.cpp"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/jsapi.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jsarray.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jsbool.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jsdate.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jsexn.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jsfriendapi.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jsmath.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jsnum.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jsutil.cpp")

if(MOZ_INSTRUMENTS)
  list(APPEND JS_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/devtools/Instrument.cpp")
endif()

if(ENABLE_TRACE_LOGGING)
  list(APPEND JS_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/TraceLogging.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/TraceLoggingGraph.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/vm/TraceLoggingTypes.cpp")
endif()

if(MOZJS_ENABLE_ION)
  list(APPEND JS_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x86-shared/Architecture-x86-shared.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x86-shared/Assembler-x86-shared.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x86-shared/AssemblerBuffer-x86-shared.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x86-shared/BaselineCompiler-x86-shared.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x86-shared/BaselineIC-x86-shared.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x86-shared/CodeGenerator-x86-shared.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x86-shared/Disassembler-x86-shared.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x86-shared/Lowering-x86-shared.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x86-shared/MacroAssembler-x86-shared.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x86-shared/MoveEmitter-x86-shared.cpp")

  if(JS_CODEGEN_X64)
    list(APPEND JS_SRCS
      "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x64/Assembler-x64.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x64/Bailouts-x64.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x64/BaselineCompiler-x64.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x64/BaselineIC-x64.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x64/CodeGenerator-x64.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x64/Lowering-x64.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x64/MacroAssembler-x64.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x64/SharedIC-x64.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x64/Trampoline-x64.cpp")
  else()
    list(APPEND JS_SRCS
      "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x86/Assembler-x86.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x86/Bailouts-x86.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x86/BaselineCompiler-x86.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x86/BaselineIC-x86.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x86/CodeGenerator-x86.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x86/Lowering-x86.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x86/MacroAssembler-x86.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x86/SharedIC-x86.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/x86/Trampoline-x86.cpp")
  endif()
else()
  list(APPEND JS_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/none/Trampoline-none.cpp")
endif()

list(APPEND JS_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/threading/windows/CpuCount.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/threading/windows/Thread.cpp")

list(APPEND JS_SRCS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/perf/pm_stub.cpp")
## End source files
###############################################################################
## Begin static library
add_library(js ${LIBRARY_TYPE} ${JS_SRCS})
add_library(mozjs::js ALIAS js)

set(JS_PUBLIC_HDRS
  "${CMAKE_CURRENT_SOURCE_DIR}/public/AllocPolicy.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/CallArgs.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/CallNonGenericMethod.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/CharacterEncoding.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/Class.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/Conversions.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/Date.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/Debug.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/GCAnnotations.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/GCAPI.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/GCHashTable.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/GCPolicyAPI.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/GCVariant.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/GCVector.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/HashTable.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/HeapAPI.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/Id.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/Initialization.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/MemoryMetrics.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/Principals.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/Printf.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/ProfilingFrameIterator.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/ProfilingStack.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/ProtoKey.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/Proxy.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/Realm.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/RefCounted.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/RequiredDefines.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/Result.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/RootingAPI.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/SliceBudget.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/Stream.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/StructuredClone.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/SweepingAPI.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/TraceKind.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/TracingAPI.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/TrackedOptimizationInfo.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/TypeDecls.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/UbiNode.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/UbiNodeBreadthFirst.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/UbiNodeCensus.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/UbiNodeDominatorTree.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/UbiNodePostOrder.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/UbiNodeShortestPaths.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/UniquePtr.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/Utility.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/Value.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/Vector.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/WeakMapPtr.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/public/Wrapper.h")

foreach(_header ${JS_PUBLIC_HDRS})
  get_filename_component(_header_file ${_header} NAME)
  configure_file(${_header} ${MOZJS_EXPORT_PREFIX}/js/${_header_file} COPYONLY)
endforeach()

set(JS_EXPORT_HDRS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/perf/jsperf.h"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/js.msg"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jsapi.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jsfriendapi.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jspubtd.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jstypes.h")

foreach(_header ${JS_EXPORT_HDRS})
  get_filename_component(_header_file ${_header} NAME)
  configure_file(${_header} ${MOZJS_EXPORT_PREFIX}/${_header_file} COPYONLY)
endforeach()

target_include_directories(js
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/assembler"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/assembler/wtf"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/assembler/jit"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ds"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/frontend"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/perf"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/yarr"
  PUBLIC
    # Binary dir public because of shell.exe...
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${MOZJS_EXPORT_PREFIX}")

target_link_libraries(js PRIVATE
    psapi

    mozjs::intl
  PUBLIC
    mozjs::fallible
    mozjs::fdlibm
    mozjs::mozalloc
    mozjs::mozmemory
    mozjs::mfbt
    mozjs::mozglue
    mozjs::zlib)

if(MOZJS_THREADSAFE)
  target_link_libraries(js PUBLIC mozjs::nspr)
endif()

# disable all warnings
target_compile_options(js PRIVATE /W0)

target_compile_definitions(js PUBLIC
  -DIMPL_MFBT
  -D_CRT_RAND_S
  -DHAVE_LOG2)
if(MOZJS_STATIC_LIB)
  target_compile_definitions(js PUBLIC -DSTATIC_JS_API)
else()
  target_compile_definitions(js PRIVATE -DEXPORT_JS_API)
endif()

if(JS_PUNBOX64)
  target_compile_definitions(js PUBLIC -DJS_CODEGEN_X64 -DJS_64BIT -DWASM_HUGE_MEMORY -DJS_CPU_X64)
elseif(JS_NUNBOX32)
  target_compile_definitions(js PUBLIC -DJS_CODEGEN_X86 -DJS_CPU_X86)
endif()

if(ENABLE_TRACE_LOGGING)
  target_compile_definitions(js PUBLIC -DJS_TRACE_LOGGING)
endif()

###############################################################################
## SpiderMonkey (Generate files)
function(GeneratedFile FILENAME)
  set(_options)
  set(_singleargs SCRIPT ENTRYPOINT)
  set(_multiargs INPUTS)

  cmake_parse_arguments(mozjs_generate "${_options}" "${_singleargs}" "${_multiargs}" "${ARGN}")
  get_filename_component(mozjs_generate_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}" DIRECTORY)

  #message(STATUS "GeneratedFile ${CMAKE_CURRENT_SOURCE_DIR}")
  #message(STATUS "script: ${mozjs_generate_SCRIPT}")
  #message(STATUS "entrypoint: ${mozjs_generate_ENTRYPOINT}")
  #message(STATUS "inputs: ${mozjs_generate_INPUTS}")
  #message(STATUS "unparsed: ${mozjs_generate_UNPARSED_ARGUMENTS}")
  #message(STATUS "directory: ${mozjs_generate_DIRECTORY}")

  # create directory for the output file
  file(MAKE_DIRECTORY ${mozjs_generate_DIRECTORY})

  set(mozjs_generate_NAMES)
  foreach(arg ${mozjs_generate_UNPARSED_ARGUMENTS})
    list(APPEND mozjs_generate_NAMES ${CMAKE_CURRENT_BINARY_DIR}/${arg})
  endforeach()
  #add_custom_command(
  #  OUTPUT ${mozjs_generate_UNPARSED_ARGUMENTS}
  #  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  #  COMMAND py -c "from ${mozjs_generate_SCRIPT} import *; ${mozjs_generate_ENTRYPOINT}(${mozjs_generate_INPUTS})"
  #  COMMENT "Generating ${mozjs_generate_UNPARSED_ARGUMENTS}"
  #  VERBATIM)
  
  # Only execute python if the file does not exist
  # This due to the fact that running python script will require it to rebuild the project
  # If the file already exists there is no need to generate and we can skip rebuild
  # This also means that changing any of the auto-generated files needs to be followed by a cache reset
  if (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}")
      if ("${mozjs_generate_ENTRYPOINT}" STREQUAL "")
        message(STATUS "Generating ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}")
        execute_process(
          OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND py ${mozjs_generate_SCRIPT} ${mozjs_generate_INPUTS})
      else()
        if ("${mozjs_generate_INPUTS}" STREQUAL "")
          message(STATUS "Generating ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}")
          execute_process(
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMAND py -c "from ${mozjs_generate_SCRIPT} import *;\nwith open('${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}', 'w') as out:\n\t\t${mozjs_generate_ENTRYPOINT}(out)")
        else()
          message(STATUS "Generating ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}")
          execute_process(
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMAND py -c "from ${mozjs_generate_SCRIPT} import *;\nwith open('${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}', 'w') as out:\n\t\t${mozjs_generate_ENTRYPOINT}(out, '${mozjs_generate_INPUTS}')")
        endif()
      endif()
  endif()

  set_source_files_properties(${mozjs_generate_UNPARSED_ARGUMENTS} PROPERTIES GENERATED TRUE)
endfunction()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/js-config.h.in ${MOZJS_EXPORT_PREFIX}/js-config.h)

set(SELFHOSTING_SRCS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/TypedObjectConstants.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/SelfHostingDefines.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/Utilities.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/Array.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/AsyncIteration.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/Classes.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/Date.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/Error.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/Function.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/Generator.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/intl/Collator.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/intl/CommonFunctions.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/intl/CurrencyDataGenerated.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/intl/DateTimeFormat.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/intl/IntlObject.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/intl/LangTagMappingsGenerated.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/intl/NumberFormat.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/intl/PluralRules.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/intl/RelativeTimeFormat.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/Iterator.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/Map.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/Module.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/Number.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/Object.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/Promise.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/Reflect.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/RegExp.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/RegExpGlobalReplaceOpt.h.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/RegExpLocalReplaceOpt.h.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/String.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/Set.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/Sorting.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/TypedArray.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/TypedObject.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/WeakMap.js"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin/WeakSet.js")

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/selfhosted.out.h
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND py src/builtin/embedjs.py -DDEBUG=1 -c "cl" -p "/P /Fi: " -m src/js.msg -o ${CMAKE_CURRENT_BINARY_DIR}/selfhosted.out.h -s ${CMAKE_CURRENT_BINARY_DIR}/selfhosted.js ${SELFHOSTING_SRCS}
  DEPENDS ${SELFHOSTING_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/src/js.msg
  COMMENT "Generating selfhosted.out.h"
  VERBATIM)

set_source_files_properties(selfhosted.out.h PROPERTIES GENERATED TRUE)
target_sources(js PRIVATE selfhosted.out.h)

#add_custom_command(
#  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/frontend/ReservedWordsGenerated.h
#  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#  COMMAND py src/frontend/GenerateReservedWords.py src/frontend/ReservedWords.h > ${CMAKE_CURRENT_BINARY_DIR}/frontend/ReservedWordsGenerated.h
#  COMMENT "Generating frontend/ReservedWordsGenerated.h"
#  VERBATIM)
#
#set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/frontend/ReservedWordsGenerated.h PROPERTIES GENERATED TRUE)
#target_sources(js PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/frontend/ReservedWordsGenerated.h)

# Generate frontend/ReservedWordsGenerated.h from frontend/ReservedWords.h
GeneratedFile(
  "frontend/ReservedWordsGenerated.h"
  SCRIPT "src/frontend/GenerateReservedWords.py"
  INPUTS "src/frontend/ReservedWords.h"
)

#add_custom_command(
#  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/gc/StatsPhasesGenerated.h
#  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#  COMMAND py src/gc/GenerateStatsPhases.py generateHeader ${CMAKE_CURRENT_BINARY_DIR}/gc/StatsPhasesGenerated.h
#  COMMENT "Generating gc/StatsPhasesGenerated.h"
#  VERBATIM)
#
#set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/gc/StatsPhasesGenerated.h PROPERTIES GENERATED TRUE)
#target_sources(js PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/gc/StatsPhasesGenerated.h)

# Generate GC statistics phase data.
GeneratedFile(
  "gc/StatsPhasesGenerated.h"
  SCRIPT "src.gc.GenerateStatsPhases"
  ENTRYPOINT "generateHeader"
)

#add_custom_command(
#  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/gc/StatsPhasesGenerated.cpp
#  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#  COMMAND py src/gc/GenerateStatsPhases.py generateCpp ${CMAKE_CURRENT_BINARY_DIR}/gc/StatsPhasesGenerated.cpp
#  COMMENT "Generating gc/StatsPhasesGenerated.cpp"
#  VERBATIM)
#
#set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/gc/StatsPhasesGenerated.cpp PROPERTIES GENERATED TRUE)
#target_sources(js PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/gc/StatsPhasesGenerated.cpp)

GeneratedFile(
  "gc/StatsPhasesGenerated.cpp"
  SCRIPT "src.gc.GenerateStatsPhases"
  ENTRYPOINT "generateCpp"
)

###############################################################################
## Begin install directives
if (MOZJS_INSTALL)
  install(TARGETS js
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${LIBRARY_DIR})
  install(DIRECTORY "${MOZJS_EXPORT_PREFIX}"
    DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()
