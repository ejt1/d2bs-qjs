cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(mozjs)

include("cmake/MozExport.cmake")

#####################################################################
## Options
option(MOZJS_INSTALL "Include install directives for mozjs" OFF)
option(MOZJS_THREADSAFE "Build SpiderMonkey with thread-safety" ON)
option(MOZJS_ENABLE_ION "Build SpiderMonkey with method-jit" ON)
option(MOZJS_SM_PROMISE "Build SpiderMonkey with Promise" ON)
option(MOZJS_STATIC_LIB "Build SpiderMonkey as a static library" ON)
option(MOZJS_STATIC_RTL "Link msvc runtime libraries statically" ON)

#####################################################################
## Compile definitions
add_definitions(-DWIN32 -D_WIN32)
add_definitions(-DXP_WIN)
add_definitions(-DWINVER=0x601)

# Make sure debug/release defines
if(CMAKE_BUILD_TYPE MATCHES "^(D|d)ebug$")
  add_definitions(-DDEBUG -D_DEBUG)
else()
  add_definitions(-DNDEBUG -D_NDEBUG)
endif()

# CPU arch
if(CMAKE_SIZEOF_VOID_P MATCHES 8)
  set(TARGET_CPU "x86_64")
  add_definitions(-D_AMD64_)
else()
  set(TARGET_CPU "x86")
  add_definitions(-D_X86_)
endif()

# option MOZJS_THREADSAFE
if(MOZJS_THREADSAFE)
  #set(JS_THREADSAFE ON)
  #add_definitions(-DJS_THREADSAFE)
  message(STATUS "Building SpiderMonkey with thread-safety ON")
endif()

if(MOZJS_ENABLE_ION)
  message(STATUS "Building SpiderMonkey with method-jit ON")
endif()

if(MOZJS_SM_PROMISE)
  message(STATUS "Building SpiderMonkey with Promise ON")
endif()

# option MOZJS_STATIC_RTL
if(MOZJS_STATIC_RTL)
  # Set runtime linking to /MT
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  message(STATUS "Link against static runtime")
endif()

# TODO: Properly define version
add_definitions(-DMOZILLA_VERSION="0.0")

# Add the ESR version to directories so we don't override when doing multiple ESR builds
set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/mozjs)
if(MOZJS_STATIC_LIB)
  set(LIBRARY_TYPE)
  set(LIBRARY_DIR lib)
  message(STATUS "Building static libraries")
  message(STATUS "Library install directory ${CMAKE_INSTALL_PREFIX}/lib")
else()
  set(LIBRARY_TYPE SHARED)
  set(LIBRARY_DIR bin)
  message(STATUS "Building dynamic libraries")
  message(STATUS "Library install directory ${CMAKE_INSTALL_PREFIX}/bin")
endif()

set(MOZJS_EXPORT_PREFIX "${CMAKE_BINARY_DIR}/mozjs/include")
message(STATUS "Exporting public headers to ${MOZJS_EXPORT_PREFIX}")

set(MOZJS_TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# BUG: JSGC_INCREMENTAL must be defined because it is used without checking if defined in jsgc.cpp#4511
add_definitions(-DJSGC_INCREMENTAL)
add_definitions(-DNOMINMAX)

# Forced ON because wont compile without
set(ENABLE_TRACE_LOGGING ON)
set(MOZJS_ENABLE_ION ON)

MOZ_EXPORT(/ FILES
  "memory/build/malloc_decls.h"
  "memory/build/mozjemalloc_types.h"
  "memory/build/mozmemory.h"
  "memory/build/mozmemory_wrap.h"
  "modules/fdlibm/src/fdlibm.h"
  "modules/zlib/src/mozzconf.h"
  "modules/zlib/src/zconf.h"
  "modules/zlib/src/zlib.h"
  "js/src/perf/jsperf.h"
  "js/src/js.msg"
  "js/src/jsapi.h"
  "js/src/jsfriendapi.h"
  "js/src/jspubtd.h"
  "js/src/jstypes.h")

MOZ_EXPORT(/double-conversion FILES
  "mfbt/double-conversion/double-conversion.h"
  "mfbt/double-conversion/utils.h")

MOZ_EXPORT(/js FILES
  "js/public/AllocPolicy.h"
  "js/public/CallArgs.h"
  "js/public/CallNonGenericMethod.h"
  "js/public/CharacterEncoding.h"
  "js/public/Class.h"
  "js/public/Conversions.h"
  "js/public/Date.h"
  "js/public/Debug.h"
  "js/public/GCAnnotations.h"
  "js/public/GCAPI.h"
  "js/public/GCHashTable.h"
  "js/public/GCPolicyAPI.h"
  "js/public/GCVariant.h"
  "js/public/GCVector.h"
  "js/public/HashTable.h"
  "js/public/HeapAPI.h"
  "js/public/Id.h"
  "js/public/Initialization.h"
  "js/public/MemoryMetrics.h"
  "js/public/Principals.h"
  "js/public/Printf.h"
  "js/public/ProfilingFrameIterator.h"
  "js/public/ProfilingStack.h"
  "js/public/ProtoKey.h"
  "js/public/Proxy.h"
  "js/public/Realm.h"
  "js/public/RefCounted.h"
  "js/public/RequiredDefines.h"
  "js/public/Result.h"
  "js/public/RootingAPI.h"
  "js/public/SliceBudget.h"
  "js/public/Stream.h"
  "js/public/StructuredClone.h"
  "js/public/SweepingAPI.h"
  "js/public/TraceKind.h"
  "js/public/TracingAPI.h"
  "js/public/TrackedOptimizationInfo.h"
  "js/public/TypeDecls.h"
  "js/public/UbiNode.h"
  "js/public/UbiNodeBreadthFirst.h"
  "js/public/UbiNodeCensus.h"
  "js/public/UbiNodeDominatorTree.h"
  "js/public/UbiNodePostOrder.h"
  "js/public/UbiNodeShortestPaths.h"
  "js/public/UniquePtr.h"
  "js/public/Utility.h"
  "js/public/Value.h"
  "js/public/Vector.h"
  "js/public/WeakMapPtr.h"
  "js/public/Wrapper.h")

MOZ_EXPORT(/mozilla FILES
  "memory/fallible/fallible.h"
  "memory/mozalloc/mozalloc.h"
  "memory/mozalloc/mozalloc_abort.h"
  "memory/mozalloc/mozalloc_oom.h"
  "mfbt/Alignment.h"
  "mfbt/AllocPolicy.h"
  "mfbt/AlreadyAddRefed.h"
  "mfbt/Array.h"
  "mfbt/ArrayUtils.h"
  "mfbt/Assertions.h"
  "mfbt/Atomics.h"
  "mfbt/Attributes.h"
  "mfbt/BinarySearch.h"
  "mfbt/BloomFilter.h"
  "mfbt/BufferList.h"
  "mfbt/Casting.h"
  "mfbt/ChaosMode.h"
  "mfbt/Char16.h"
  "mfbt/CheckedInt.h"
  "mfbt/Compiler.h"
  "mfbt/Compression.h"
  "mfbt/DebugOnly.h"
  "mfbt/DefineEnum.h"
  "mfbt/DoublyLinkedList.h"
  "mfbt/EndianUtils.h"
  "mfbt/EnumeratedArray.h"
  "mfbt/EnumeratedRange.h"
  "mfbt/EnumSet.h"
  "mfbt/EnumTypeTraits.h"
  "mfbt/FastBernoulliTrial.h"
  "mfbt/FloatingPoint.h"
  "mfbt/FStream.h"
  "mfbt/GuardObjects.h"
  "mfbt/HashFunctions.h"
  "mfbt/IndexSequence.h"
  "mfbt/IntegerPrintfMacros.h"
  "mfbt/IntegerRange.h"
  "mfbt/IntegerTypeTraits.h"
  "mfbt/JSONWriter.h"
  "mfbt/Likely.h"
  "mfbt/LinkedList.h"
  "mfbt/LinuxSignal.h"
  "mfbt/MacroArgs.h"
  "mfbt/MacroForEach.h"
  "mfbt/MathAlgorithms.h"
  "mfbt/Maybe.h"
  "mfbt/MaybeOneOf.h"
  "mfbt/MemoryChecking.h"
  "mfbt/MemoryReporting.h"
  "mfbt/Move.h"
  "mfbt/NullPtr.h"
  "mfbt/Opaque.h"
  "mfbt/OperatorNewExtensions.h"
  "mfbt/Pair.h"
  "mfbt/PodOperations.h"
  "mfbt/Poison.h"
  "mfbt/Range.h"
  "mfbt/RangedArray.h"
  "mfbt/RangedPtr.h"
  "mfbt/ReentrancyGuard.h"
  "mfbt/RefCounted.h"
  "mfbt/RefCountType.h"
  "mfbt/RefPtr.h"
  "mfbt/Result.h"
  "mfbt/ResultExtensions.h"
  "mfbt/ReverseIterator.h"
  "mfbt/RollingMean.h"
  "mfbt/Saturate.h"
  "mfbt/Scoped.h"
  "mfbt/ScopeExit.h"
  "mfbt/SegmentedVector.h"
  "mfbt/SHA1.h"
  "mfbt/SmallPointerArray.h"
  "mfbt/Span.h"
  "mfbt/SplayTree.h"
  "mfbt/Sprintf.h"
  "mfbt/StaticAnalysisFunctions.h"
  "mfbt/TaggedAnonymousMemory.h"
  "mfbt/TemplateLib.h"
  "mfbt/TextUtils.h"
  "mfbt/ThreadLocal.h"
  "mfbt/ThreadSafeWeakPtr.h"
  "mfbt/ToString.h"
  "mfbt/Tuple.h"
  "mfbt/TypedEnumBits.h"
  "mfbt/Types.h"
  "mfbt/TypeTraits.h"
  "mfbt/UniquePtr.h"
  "mfbt/UniquePtrExtensions.h"
  "mfbt/unused.h"
  "mfbt/Variant.h"
  "mfbt/Vector.h"
  "mfbt/WeakPtr.h"
  "mfbt/WindowsVersion.h"
  "mfbt/WrappingOperations.h"
  "mfbt/XorShift128PlusRNG.h"
  "mozglue/misc/AutoProfilerLabel.h"
  "mozglue/misc/PlatformConditionVariable.h"
  "mozglue/misc/PlatformMutex.h"
  "mozglue/misc/Printf.h"
  "mozglue/misc/StackWalk.h"
  "mozglue/misc/StackWalk_windows.h"
  "mozglue/misc/TimeStamp.h"
  "mozglue/misc/TimeStamp_windows.h")

MOZ_EXPORT(/mozilla/decimal FILES
  "mfbt/decimal/Decimal.h")

set(MOZJS_SOURCES
  "memory/build/fallback.cpp"
  "memory/mozalloc/mozalloc.cpp"
  "memory/mozalloc/mozalloc_abort.cpp"
  "memory/mozalloc/mozalloc_oom.cpp"
  "memory/mozalloc/winheap.cpp"
  "mfbt/double-conversion/bignum.cc"
  "mfbt/double-conversion/bignum-dtoa.cc"
  "mfbt/double-conversion/cached-powers.cc"
  "mfbt/double-conversion/diy-fp.cc"
  "mfbt/double-conversion/double-conversion.cc"
  "mfbt/double-conversion/fast-dtoa.cc"
  "mfbt/double-conversion/fixed-dtoa.cc"
  "mfbt/double-conversion/strtod.cc"
  "mfbt/decimal/Decimal.cpp"
  "mfbt/Assertions.cpp"
  "mfbt/ChaosMode.cpp"
  "mfbt/Compression.cpp"
  "mfbt/FloatingPoint.cpp"
  "mfbt/HashFunctions.cpp"
  "mfbt/JSONWriter.cpp"
  "mfbt/lz4.c"
  "mfbt/Poison.cpp"
  "mfbt/SHA1.cpp"
  "mfbt/TaggedAnonymousMemory.cpp"
  "mfbt/unused.cpp"
  "modules/fdlibm/src/e_acos.cpp"
  "modules/fdlibm/src/e_acosh.cpp"
  "modules/fdlibm/src/e_asin.cpp"
  "modules/fdlibm/src/e_atan2.cpp"
  "modules/fdlibm/src/e_atanh.cpp"
  "modules/fdlibm/src/e_cosh.cpp"
  "modules/fdlibm/src/e_exp.cpp"
  "modules/fdlibm/src/e_hypot.cpp"
  "modules/fdlibm/src/e_log.cpp"
  "modules/fdlibm/src/e_log10.cpp"
  "modules/fdlibm/src/e_log2.cpp"
  "modules/fdlibm/src/e_pow.cpp"
  "modules/fdlibm/src/e_sinh.cpp"
  "modules/fdlibm/src/e_sqrt.cpp"
  "modules/fdlibm/src/k_exp.cpp"
  "modules/fdlibm/src/s_asinh.cpp"
  "modules/fdlibm/src/s_atan.cpp"
  "modules/fdlibm/src/s_cbrt.cpp"
  "modules/fdlibm/src/s_ceil.cpp"
  "modules/fdlibm/src/s_ceilf.cpp"
  "modules/fdlibm/src/s_copysign.cpp"
  "modules/fdlibm/src/s_expm1.cpp"
  "modules/fdlibm/src/s_fabs.cpp"
  "modules/fdlibm/src/s_floor.cpp"
  "modules/fdlibm/src/s_floorf.cpp"
  "modules/fdlibm/src/s_log1p.cpp"
  "modules/fdlibm/src/s_nearbyint.cpp"
  "modules/fdlibm/src/s_rint.cpp"
  "modules/fdlibm/src/s_rintf.cpp"
  "modules/fdlibm/src/s_scalbn.cpp"
  "modules/fdlibm/src/s_tanh.cpp"
  "modules/fdlibm/src/s_trunc.cpp"
  "modules/fdlibm/src/s_truncf.cpp"
  "modules/zlib/src/adler32.c"
  "modules/zlib/src/compress.c"
  "modules/zlib/src/crc32.c"
  "modules/zlib/src/deflate.c"
  "modules/zlib/src/gzclose.c"
  "modules/zlib/src/gzlib.c"
  "modules/zlib/src/gzread.c"
  "modules/zlib/src/gzwrite.c"
  "modules/zlib/src/infback.c"
  "modules/zlib/src/inffast.c"
  "modules/zlib/src/inflate.c"
  "modules/zlib/src/inftrees.c"
  "modules/zlib/src/trees.c"
  "modules/zlib/src/uncompr.c"
  "modules/zlib/src/zutil.c"
  "mozglue/misc/AutoProfilerLabel.cpp"
  "mozglue/misc/ConditionVariable_windows.cpp"
  "mozglue/misc/Mutex_windows.cpp"
  "mozglue/misc/Printf.cpp"
  "mozglue/misc/StackWalk.cpp"
  "mozglue/misc/TimeStamp.cpp"
  "mozglue/misc/TimeStamp_windows.cpp"
  "js/src/builtin/intl/Collator.cpp"
  "js/src/builtin/intl/CommonFunctions.cpp"
  "js/src/builtin/intl/DateTimeFormat.cpp"
  "js/src/builtin/intl/IntlObject.cpp"
  "js/src/builtin/intl/NumberFormat.cpp"
  "js/src/builtin/intl/PluralRules.cpp"
  "js/src/builtin/intl/RelativeTimeFormat.cpp"
  "js/src/builtin/intl/SharedIntlData.cpp"
  "js/src/builtin/AtomicsObject.cpp"
  "js/src/builtin/DataViewObject.cpp"
  "js/src/builtin/Eval.cpp"
  "js/src/builtin/JSON.cpp"
  "js/src/builtin/MapObject.cpp"
  "js/src/builtin/ModuleObject.cpp"
  "js/src/builtin/Object.cpp"
  "js/src/builtin/Profilers.cpp"
  "js/src/builtin/Promise.cpp"
  "js/src/builtin/Reflect.cpp"
  "js/src/builtin/ReflectParse.cpp"
  "js/src/builtin/RegExp.cpp"
  "js/src/builtin/SIMD.cpp"
  "js/src/builtin/Stream.cpp"
  "js/src/builtin/String.cpp"
  "js/src/builtin/Symbol.cpp"
  "js/src/builtin/TestingFunctions.cpp"
  "js/src/builtin/TypedObject.cpp"
  "js/src/builtin/WeakMapObject.cpp"
  "js/src/builtin/WeakSetObject.cpp"
  "js/src/devtools/sharkctl.cpp"
  "js/src/ds/Bitmap.cpp"
  "js/src/ds/LifoAlloc.cpp"
  "js/src/ds/MemoryProtectionExceptionHandler.cpp"
  "js/src/frontend/BytecodeCompiler.cpp"
  "js/src/frontend/BytecodeEmitter.cpp"
  "js/src/frontend/FoldConstants.cpp"
  "js/src/frontend/NameFunctions.cpp"
  "js/src/frontend/ParseNode.cpp"
  "js/src/frontend/Parser.cpp"
  "js/src/frontend/TokenStream.cpp"
  "js/src/gc/Allocator.cpp"
  "js/src/gc/AtomMarking.cpp"
  "js/src/gc/Barrier.cpp"
  "js/src/gc/GC.cpp"
  "js/src/gc/GCTrace.cpp"
  "js/src/gc/Marking.cpp"
  "js/src/gc/Memory.cpp"
  "js/src/gc/Nursery.cpp"
  "js/src/gc/PublicIterators.cpp"
  "js/src/gc/RootMarking.cpp"
  "js/src/gc/Statistics.cpp"
  "js/src/gc/StoreBuffer.cpp"
  "js/src/gc/Tracer.cpp"
  "js/src/gc/Verifier.cpp"
  "js/src/gc/WeakMap.cpp"
  "js/src/gc/WeakMapPtr.cpp"
  "js/src/gc/Zone.cpp"
  "js/src/gc/ZoneGroup.cpp"
  "js/src/irregexp/NativeRegExpMacroAssembler.cpp"
  "js/src/irregexp/RegExpAST.cpp"
  "js/src/irregexp/RegExpCharacters.cpp"
  "js/src/irregexp/RegExpEngine.cpp"
  "js/src/irregexp/RegExpInterpreter.cpp"
  "js/src/irregexp/RegExpMacroAssembler.cpp"
  "js/src/irregexp/RegExpParser.cpp"
  "js/src/irregexp/RegExpStack.cpp"
  "js/src/jit/AliasAnalysis.cpp"
  "js/src/jit/AliasAnalysisShared.cpp"
  "js/src/jit/AlignmentMaskAnalysis.cpp"
  "js/src/jit/BacktrackingAllocator.cpp"
  "js/src/jit/Bailouts.cpp"
  "js/src/jit/BaselineBailouts.cpp"
  "js/src/jit/BaselineCacheIRCompiler.cpp"
  "js/src/jit/BaselineCompiler.cpp"
  "js/src/jit/BaselineDebugModeOSR.cpp"
  "js/src/jit/BaselineFrame.cpp"
  "js/src/jit/BaselineFrameInfo.cpp"
  "js/src/jit/BaselineIC.cpp"
  "js/src/jit/BaselineInspector.cpp"
  "js/src/jit/BaselineJIT.cpp"
  "js/src/jit/BitSet.cpp"
  "js/src/jit/BytecodeAnalysis.cpp"
  "js/src/jit/C1Spewer.cpp"
  "js/src/jit/CacheIR.cpp"
  "js/src/jit/CacheIRCompiler.cpp"
  "js/src/jit/CacheIRSpewer.cpp"
  "js/src/jit/CodeGenerator.cpp"
  "js/src/jit/CompileWrappers.cpp"
  "js/src/jit/Disassembler.cpp"
  "js/src/jit/EagerSimdUnbox.cpp"
  "js/src/jit/EdgeCaseAnalysis.cpp"
  "js/src/jit/EffectiveAddressAnalysis.cpp"
  "js/src/jit/ExecutableAllocator.cpp"
  "js/src/jit/FlowAliasAnalysis.cpp"
  "js/src/jit/FoldLinearArithConstants.cpp"
  "js/src/jit/InstructionReordering.cpp"
  "js/src/jit/Ion.cpp"
  "js/src/jit/IonAnalysis.cpp"
  "js/src/jit/IonBuilder.cpp"
  "js/src/jit/IonCacheIRCompiler.cpp"
  "js/src/jit/IonControlFlow.cpp"
  "js/src/jit/IonIC.cpp"
  "js/src/jit/IonOptimizationLevels.cpp"
  "js/src/jit/Jit.cpp"
  "js/src/jit/JitcodeMap.cpp"
  "js/src/jit/JitFrames.cpp"
  "js/src/jit/JitOptions.cpp"
  "js/src/jit/JitSpewer.cpp"
  "js/src/jit/JSJitFrameIter.cpp"
  "js/src/jit/JSONSpewer.cpp"
  "js/src/jit/LICM.cpp"
  "js/src/jit/Linker.cpp"
  "js/src/jit/LIR.cpp"
  "js/src/jit/LoopUnroller.cpp"
  "js/src/jit/Lowering.cpp"
  "js/src/jit/MacroAssembler.cpp"
  "js/src/jit/MCallOptimize.cpp"
  "js/src/jit/MIR.cpp"
  "js/src/jit/MIRGraph.cpp"
  "js/src/jit/MoveResolver.cpp"
  "js/src/jit/OptimizationTracking.cpp"
  "js/src/jit/PerfSpewer.cpp"
  "js/src/jit/ProcessExecutableMemory.cpp"
  "js/src/jit/RangeAnalysis.cpp"
  "js/src/jit/Recover.cpp"
  "js/src/jit/RegisterAllocator.cpp"
  "js/src/jit/RematerializedFrame.cpp"
  "js/src/jit/Safepoints.cpp"
  "js/src/jit/ScalarReplacement.cpp"
  "js/src/jit/shared/Assembler-shared.cpp"
  "js/src/jit/shared/BaselineCompiler-shared.cpp"
  "js/src/jit/shared/CodeGenerator-shared.cpp"
  "js/src/jit/shared/Disassembler-shared.cpp"
  "js/src/jit/shared/Lowering-shared.cpp"
  "js/src/jit/SharedIC.cpp"
  "js/src/jit/Sink.cpp"
  "js/src/jit/Snapshots.cpp"
  "js/src/jit/StupidAllocator.cpp"
  "js/src/jit/TypedObjectPrediction.cpp"
  "js/src/jit/TypePolicy.cpp"
  "js/src/jit/ValueNumbering.cpp"
  "js/src/jit/VMFunctions.cpp"
  "js/src/jit/WasmBCE.cpp"
  "js/src/perf/jsperf.cpp"
  "js/src/proxy/BaseProxyHandler.cpp"
  "js/src/proxy/CrossCompartmentWrapper.cpp"
  "js/src/proxy/DeadObjectProxy.cpp"
  "js/src/proxy/OpaqueCrossCompartmentWrapper.cpp"
  "js/src/proxy/Proxy.cpp"
  "js/src/proxy/ScriptedProxyHandler.cpp"
  "js/src/proxy/SecurityWrapper.cpp"
  "js/src/proxy/Wrapper.cpp"
  "js/src/threading/Mutex.cpp"
  "js/src/threading/ProtectedData.cpp"
  "js/src/util/AllocPolicy.cpp"
  "js/src/util/DoubleToString.cpp"
  "js/src/util/NativeStack.cpp"
  "js/src/util/Printf.cpp"
  "js/src/util/StringBuffer.cpp"
  "js/src/util/Text.cpp"
  "js/src/util/Unicode.cpp"
  "js/src/vm/ArgumentsObject.cpp"
  "js/src/vm/ArrayBufferObject.cpp"
  "js/src/vm/AsyncFunction.cpp"
  "js/src/vm/AsyncIteration.cpp"
  "js/src/vm/BytecodeUtil.cpp"
  "js/src/vm/Caches.cpp"
  "js/src/vm/CallNonGenericMethod.cpp"
  "js/src/vm/CharacterEncoding.cpp"
  "js/src/vm/CodeCoverage.cpp"
  "js/src/vm/Compression.cpp"
  "js/src/vm/DateTime.cpp"
  "js/src/vm/Debugger.cpp"
  "js/src/vm/DebuggerMemory.cpp"
  "js/src/vm/EnvironmentObject.cpp"
  "js/src/vm/ErrorObject.cpp"
  "js/src/vm/ErrorReporting.cpp"
  "js/src/vm/ForOfIterator.cpp"
  "js/src/vm/GeckoProfiler.cpp"
  "js/src/vm/GeneratorObject.cpp"
  "js/src/vm/GlobalObject.cpp"
  "js/src/vm/HelperThreads.cpp"
  "js/src/vm/Id.cpp"
  "js/src/vm/Initialization.cpp"
  "js/src/vm/Interpreter.cpp"
  "js/src/vm/Iteration.cpp"
  "js/src/vm/JSAtom.cpp"
  "js/src/vm/JSCompartment.cpp"
  "js/src/vm/JSContext.cpp"
  "js/src/vm/JSFunction.cpp"
  "js/src/vm/JSObject.cpp"
  "js/src/vm/JSONParser.cpp"
  "js/src/vm/JSONPrinter.cpp"
  "js/src/vm/JSScript.cpp"
  "js/src/vm/MemoryMetrics.cpp"
  "js/src/vm/NativeObject.cpp"
  "js/src/vm/ObjectGroup.cpp"
  "js/src/vm/PIC.cpp"
  "js/src/vm/Printer.cpp"
  "js/src/vm/Probes.cpp"
  "js/src/vm/ProxyObject.cpp"
  "js/src/vm/Realm.cpp"
  "js/src/vm/ReceiverGuard.cpp"
  "js/src/vm/RegExpObject.cpp"
  "js/src/vm/RegExpStatics.cpp"
  "js/src/vm/Runtime.cpp"
  "js/src/vm/SavedStacks.cpp"
  "js/src/vm/Scope.cpp"
  "js/src/vm/SelfHosting.cpp"
  "js/src/vm/Shape.cpp"
  "js/src/vm/SharedArrayObject.cpp"
  "js/src/vm/SharedImmutableStringsCache.cpp"
  "js/src/vm/Stack.cpp"
  "js/src/vm/Stopwatch.cpp"
  "js/src/vm/StringType.cpp"
  "js/src/vm/StructuredClone.cpp"
  "js/src/vm/SymbolType.cpp"
  "js/src/vm/TaggedProto.cpp"
  "js/src/vm/Time.cpp"
  "js/src/vm/TypedArrayObject.cpp"
  "js/src/vm/TypeInference.cpp"
  "js/src/vm/UbiNode.cpp"
  "js/src/vm/UbiNodeCensus.cpp"
  "js/src/vm/UbiNodeShortestPaths.cpp"
  "js/src/vm/UnboxedObject.cpp"
  "js/src/vm/Value.cpp"
  "js/src/vm/Xdr.cpp"
  "js/src/wasm/AsmJS.cpp"
  "js/src/wasm/WasmBaselineCompile.cpp"
  "js/src/wasm/WasmBinaryIterator.cpp"
  "js/src/wasm/WasmBinaryToAST.cpp"
  "js/src/wasm/WasmBinaryToText.cpp"
  "js/src/wasm/WasmBuiltins.cpp"
  "js/src/wasm/WasmCode.cpp"
  "js/src/wasm/WasmCompartment.cpp"
  "js/src/wasm/WasmCompile.cpp"
  "js/src/wasm/WasmDebug.cpp"
  "js/src/wasm/WasmFrameIter.cpp"
  "js/src/wasm/WasmGenerator.cpp"
  "js/src/wasm/WasmInstance.cpp"
  "js/src/wasm/WasmIonCompile.cpp"
  "js/src/wasm/WasmJS.cpp"
  "js/src/wasm/WasmModule.cpp"
  "js/src/wasm/WasmProcess.cpp"
  "js/src/wasm/WasmSignalHandlers.cpp"
  "js/src/wasm/WasmStubs.cpp"
  "js/src/wasm/WasmTable.cpp"
  "js/src/wasm/WasmTextToBinary.cpp"
  "js/src/wasm/WasmTextUtils.cpp"
  "js/src/wasm/WasmTypes.cpp"
  "js/src/wasm/WasmValidate.cpp"
  "js/src/jsapi.cpp"
  "js/src/jsarray.cpp"
  "js/src/jsbool.cpp"
  "js/src/jsdate.cpp"
  "js/src/jsexn.cpp"
  "js/src/jsfriendapi.cpp"
  "js/src/jsmath.cpp"
  "js/src/jsnum.cpp"
  "js/src/jsutil.cpp"
  "nsprpub/lib/libc/src/plvrsion.c"
  "nsprpub/lib/libc/src/strlen.c"
  "nsprpub/lib/libc/src/strcpy.c"
  "nsprpub/lib/libc/src/strdup.c"
  "nsprpub/lib/libc/src/strcase.c"
  "nsprpub/lib/libc/src/strcat.c"
  "nsprpub/lib/libc/src/strcmp.c"
  "nsprpub/lib/libc/src/strchr.c"
  "nsprpub/lib/libc/src/strpbrk.c"
  "nsprpub/lib/libc/src/strstr.c"
  "nsprpub/lib/libc/src/strtok.c"
  "nsprpub/lib/libc/src/base64.c"
  "nsprpub/lib/libc/src/plerror.c"
  "nsprpub/lib/libc/src/plgetopt.c"
  "nsprpub/pr/src/io/prdir.c"
  "nsprpub/pr/src/io/prfdcach.c"
  "nsprpub/pr/src/io/prfile.c"
  "nsprpub/pr/src/io/prio.c"
  "nsprpub/pr/src/io/priometh.c "
  "nsprpub/pr/src/io/pripv6.c"
  "nsprpub/pr/src/io/prlayer.c"
  "nsprpub/pr/src/io/prlog.c"
  "nsprpub/pr/src/io/prmapopt.c"
  "nsprpub/pr/src/io/prmmap.c"
  "nsprpub/pr/src/io/prmwait.c"
  "nsprpub/pr/src/io/prpolevt.c"
  "nsprpub/pr/src/io/prprf.c"
  "nsprpub/pr/src/io/prscanf.c"
  "nsprpub/pr/src/io/prsocket.c"
  "nsprpub/pr/src/io/prstdio.c"
  "nsprpub/pr/src/linking/prlink.c"
  "nsprpub/pr/src/malloc/prmalloc.c"
  "nsprpub/pr/src/malloc/prmem.c"
  "nsprpub/pr/src/md/windows/ntmisc.c"
  "nsprpub/pr/src/md/windows/ntsec.c"
  "nsprpub/pr/src/md/windows/ntsem.c"
  "nsprpub/pr/src/md/windows/ntinrval.c"
  "nsprpub/pr/src/md/windows/ntgc.c"
  "nsprpub/pr/src/md/windows/w95thred.c"
  "nsprpub/pr/src/md/windows/w95io.c"
  "nsprpub/pr/src/md/windows/w95cv.c"
  "nsprpub/pr/src/md/windows/w95sock.c"
  "nsprpub/pr/src/md/windows/win32_errors.c"
  "nsprpub/pr/src/md/windows/w32ipcsem.c"
  "nsprpub/pr/src/md/windows/w32poll.c"
  "nsprpub/pr/src/md/windows/w32rng.c"
  "nsprpub/pr/src/md/windows/w32shm.c"
  "nsprpub/pr/src/md/windows/w95dllmain.c"
  "nsprpub/pr/src/md/prosdep.c"
  "nsprpub/pr/src/memory/prseg.c"
  "nsprpub/pr/src/memory/prshm.c"
  "nsprpub/pr/src/memory/prshma.c"
  "nsprpub/pr/src/misc/pralarm.c"
  "nsprpub/pr/src/misc/pratom.c"
  "nsprpub/pr/src/misc/praton.c"
  "nsprpub/pr/src/misc/prcountr.c"
  "nsprpub/pr/src/misc/prdtoa.c"
  "nsprpub/pr/src/misc/prenv.c"
  "nsprpub/pr/src/misc/prerr.c"
  "nsprpub/pr/src/misc/prerror.c"
  "nsprpub/pr/src/misc/prerrortable.c"
  "nsprpub/pr/src/misc/prinit.c"
  "nsprpub/pr/src/misc/prinrval.c"
  "nsprpub/pr/src/misc/pripc.c"
  "nsprpub/pr/src/misc/pripcsem.c"
  "nsprpub/pr/src/misc/prlog2.c"
  "nsprpub/pr/src/misc/prlong.c"
  "nsprpub/pr/src/misc/prnetdb.c"
  "nsprpub/pr/src/misc/prolock.c"
  "nsprpub/pr/src/misc/prrng.c"
  "nsprpub/pr/src/misc/prsystem.c"
  "nsprpub/pr/src/misc/prthinfo.c"
  "nsprpub/pr/src/misc/prtime.c"
  "nsprpub/pr/src/misc/prtpool.c"
  "nsprpub/pr/src/misc/prtrace.c"
  "nsprpub/pr/src/threads/combined/prucpu.c"
  "nsprpub/pr/src/threads/combined/prucv.c"
  "nsprpub/pr/src/threads/combined/prulock.c"
  "nsprpub/pr/src/threads/combined/pruthr.c"
  "nsprpub/pr/src/threads/combined/prustack.c"
  "nsprpub/pr/src/threads/prcmon.c"
  "nsprpub/pr/src/threads/prdump.c"
  "nsprpub/pr/src/threads/prmon.c"
  "nsprpub/pr/src/threads/prsem.c"
  "nsprpub/pr/src/threads/prrwlock.c"
  "nsprpub/pr/src/threads/prcthr.c"
  "nsprpub/pr/src/threads/prtpd.c")

if(MOZ_INSTRUMENTS)
  list(APPEND MOZJS_SOURCES
    "js/src/devtools/Instrument.cpp")
endif()

if(ENABLE_TRACE_LOGGING)
  list(APPEND MOZJS_SOURCES
    "js/src/vm/TraceLogging.cpp"
    "js/src/vm/TraceLoggingGraph.cpp"
    "js/src/vm/TraceLoggingTypes.cpp")
endif()

if(MOZJS_ENABLE_ION)
  list(APPEND MOZJS_SOURCES
    "js/src/jit/x86-shared/Architecture-x86-shared.cpp"
    "js/src/jit/x86-shared/Assembler-x86-shared.cpp"
    "js/src/jit/x86-shared/AssemblerBuffer-x86-shared.cpp"
    "js/src/jit/x86-shared/BaselineCompiler-x86-shared.cpp"
    "js/src/jit/x86-shared/BaselineIC-x86-shared.cpp"
    "js/src/jit/x86-shared/CodeGenerator-x86-shared.cpp"
    "js/src/jit/x86-shared/Disassembler-x86-shared.cpp"
    "js/src/jit/x86-shared/Lowering-x86-shared.cpp"
    "js/src/jit/x86-shared/MacroAssembler-x86-shared.cpp"
    "js/src/jit/x86-shared/MoveEmitter-x86-shared.cpp")

  if(JS_CODEGEN_X64)
    list(APPEND MOZJS_SOURCES
      "js/src/jit/x64/Assembler-x64.cpp"
      "js/src/jit/x64/Bailouts-x64.cpp"
      "js/src/jit/x64/BaselineCompiler-x64.cpp"
      "js/src/jit/x64/BaselineIC-x64.cpp"
      "js/src/jit/x64/CodeGenerator-x64.cpp"
      "js/src/jit/x64/Lowering-x64.cpp"
      "js/src/jit/x64/MacroAssembler-x64.cpp"
      "js/src/jit/x64/SharedIC-x64.cpp"
      "js/src/jit/x64/Trampoline-x64.cpp")
  else()
    list(APPEND MOZJS_SOURCES
      "js/src/jit/x86/Assembler-x86.cpp"
      "js/src/jit/x86/Bailouts-x86.cpp"
      "js/src/jit/x86/BaselineCompiler-x86.cpp"
      "js/src/jit/x86/BaselineIC-x86.cpp"
      "js/src/jit/x86/CodeGenerator-x86.cpp"
      "js/src/jit/x86/Lowering-x86.cpp"
      "js/src/jit/x86/MacroAssembler-x86.cpp"
      "js/src/jit/x86/SharedIC-x86.cpp"
      "js/src/jit/x86/Trampoline-x86.cpp")
  endif()
else()
  list(APPEND MOZJS_SOURCES
    "js/src/jit/none/Trampoline-none.cpp")
endif()

list(APPEND MOZJS_SOURCES
    "js/src/threading/windows/CpuCount.cpp"
    "js/src/threading/windows/Thread.cpp")

list(APPEND MOZJS_SOURCES
  "js/src/perf/pm_stub.cpp")
  
if (MOZJS_THREADSAFE)
  set(JS_THREADSAFE ON)
endif()

set(JS_GC_ZEAL ON)
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

configure_file("js/src/js-config.h.in" ${CMAKE_BINARY_DIR}/dist/js-config.h)

add_library(mozjs ${LIBRARY_TYPE} ${MOZJS_SOURCES})

target_include_directories(mozjs
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include" # generated headers
    ${CMAKE_CURRENT_SOURCE_DIR}/mfbt
    "${CMAKE_CURRENT_SOURCE_DIR}/nsprpub/lib/libc/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/nsprpub/pr/include/md"
    "${CMAKE_CURRENT_SOURCE_DIR}/nsprpub/pr/include/obsolete"
    "${CMAKE_CURRENT_SOURCE_DIR}/nsprpub/pr/include/private"
    "${CMAKE_CURRENT_SOURCE_DIR}/nsprpub/pr/include"
    ${CMAKE_CURRENT_SOURCE_DIR}/js/src
  PUBLIC
    # public exports
    ${CMAKE_BINARY_DIR}/dist)

target_link_libraries(mozjs
  PUBLIC
    # needed by nspr
    winmm)

# disable all warnings
target_compile_options(mozjs PRIVATE /W0)

# nspr definitions
target_compile_definitions(mozjs
  PRIVATE
    -D_NSPR_BUILD_
    -D_PR_GLOBAL_THREADS_ONLY
    -DXP_PC
    -DWIN95
    -D_USE_MATH_DEFINES
  PUBLIC
    -D_NSPR_STATIC_)

if(TARGET_CPU MATCHES "^x86_64$")
  target_compile_definitions(mozjs PRIVATE
    -DUSE_64)
  message(STATUS "Build NSPR as 64-bit")
endif()

configure_file(nsprpub/pr/include/md/_win95.cfg ${CMAKE_BINARY_DIR}/dist/prcpucfg.h COPYONLY)
target_sources(mozjs PUBLIC ${CMAKE_BINARY_DIR}/dist/prcpucfg.h)

# Make sure debug/release defines
if(CMAKE_BUILD_TYPE MATCHES "^(D|d)ebug$")
  target_compile_definitions(mozjs PUBLIC -DDEBUG -D_DEBUG)
else()
  target_compile_definitions(mozjs PUBLIC -DNDEBUG -D_NDEBUG)
endif()

target_compile_definitions(mozjs PUBLIC
  -DJS_DEFAULT_JITREPORT_GRANULARITY=3
  -DIMPL_MFBT
  -D_CRT_RAND_S
  -DHAVE_LOG2)
if(MOZJS_STATIC_LIB)
  target_compile_definitions(mozjs PUBLIC -DSTATIC_JS_API)
else()
  target_compile_definitions(mozjs PRIVATE -DEXPORT_JS_API)
endif()

if(JS_PUNBOX64)
  target_compile_definitions(mozjs PUBLIC -DJS_CODEGEN_X64 -DJS_64BIT -DWASM_HUGE_MEMORY -DJS_CPU_X64)
elseif(JS_NUNBOX32)
  target_compile_definitions(mozjs PUBLIC -DJS_CODEGEN_X86 -DJS_CPU_X86)
endif()

if(ENABLE_TRACE_LOGGING)
  target_compile_definitions(mozjs PUBLIC -DJS_TRACE_LOGGING)
endif()
