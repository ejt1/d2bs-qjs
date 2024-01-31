###############################################################################
## Begin definitions
# TODO: Just temporarily define these to successfully compile and move later.
add_definitions(-DSTATIC_JS_API)
add_definitions(-DJS_DEFAULT_JITREPORT_GRANULARITY=3)
add_definitions(-DWIN32)
if(CMAKE_SIZEOF_VOID_P MATCHES 8)
  add_definitions(-D_AMD64_)
else()
  add_definitions(-D_X86_)
endif()

if(MOZJS_THREADSAFE)
  add_definitions(-DJS_THREADSAFE)
endif()
## End definitions
###############################################################################
## Begin source files
set(JS_SRCS
  src/builtin/Eval.cpp
  src/builtin/Intl.cpp
  src/builtin/MapObject.cpp
  src/builtin/Module.cpp
  src/builtin/Object.cpp
  src/builtin/ParallelArray.cpp
  src/builtin/Profilers.cpp
  src/builtin/RegExp.cpp
  src/builtin/TestingFunctions.cpp

  src/devtools/sharkctl.cpp

  src/ds/LifoAlloc.cpp
  
  src/frontend/BytecodeCompiler.cpp
  src/frontend/BytecodeEmitter.cpp
  src/frontend/FoldConstants.cpp
  src/frontend/NameFunctions.cpp
  src/frontend/ParseMaps.cpp
  src/frontend/ParseNode.cpp
  src/frontend/Parser.cpp
  src/frontend/TokenStream.cpp
  
  src/vm/ArgumentsObject.cpp
  src/vm/CharacterEncoding.cpp
  src/vm/DateTime.cpp
  src/vm/Debugger.cpp
  src/vm/ForkJoin.cpp
  src/vm/GlobalObject.cpp
  src/vm/Monitor.cpp
  src/vm/ObjectImpl.cpp
  src/vm/ParallelDo.cpp
  src/vm/PropertyKey.cpp
  src/vm/RegExpObject.cpp
  src/vm/RegExpStatics.cpp
  src/vm/ScopeObject.cpp
  src/vm/SelfHosting.cpp
  src/vm/Shape.cpp
  src/vm/SPSProfiler.cpp
  src/vm/Stack.cpp
  src/vm/String.cpp
  src/vm/StringBuffer.cpp
  src/vm/ThreadPool.cpp
  src/vm/Unicode.cpp
  src/vm/Xdr.cpp
  
  src/gc/Iteration.cpp
  src/gc/Marking.cpp
  src/gc/Memory.cpp
  src/gc/RootMarking.cpp
  src/gc/Statistics.cpp
  src/gc/StoreBuffer.cpp
  src/gc/Verifier.cpp
  src/gc/Zone.cpp
  
  src/jsalloc.cpp
  src/jsanalyze.cpp
  src/jsapi.cpp
  src/jsarray.cpp
  src/jsatom.cpp
  src/jsbool.cpp
  src/jsclone.cpp
  src/jscntxt.cpp
  src/jscompartment.cpp
  src/jscrashreport.cpp
  src/jsdate.cpp
  src/jsdbgapi.cpp
  src/jsdhash.cpp
  src/jsdtoa.cpp
  src/jsexn.cpp
  src/jsfriendapi.cpp
  src/jsfun.cpp
  src/jsgc.cpp
  src/jsinfer.cpp
  src/jsinterp.cpp
  src/jsiter.cpp
  src/jslog2.cpp
  src/jsmath.cpp
  src/jsmemorymetrics.cpp
  src/jsnativestack.cpp
  src/jsnum.cpp
  src/jsobj.cpp
  src/json.cpp
  src/jsonparser.cpp
  src/jsopcode.cpp
  src/jsprf.cpp
  src/jsprobes.cpp
  src/jspropertycache.cpp
  src/jspropertytree.cpp
  src/jsproxy.cpp
  src/jsreflect.cpp
  src/jsscript.cpp
  src/jsstr.cpp
  src/jstypedarray.cpp
  src/jsutil.cpp
  src/jswatchpoint.cpp
  src/jsweakmap.cpp
  src/jsworkers.cpp
  src/jswrapper.cpp
  src/prmjtime.cpp)

if(MOZ_INSTRUMENTS)
  list(APPEND JS_SRCS
    src/devtools/Instrument.cpp)
endif()

if(ENABLE_TRACE_LOGGING)
  list(APPEND JS_SRCS
    src/TraceLogging.cpp)
endif()

if(ENABLE_METHODJIT)
  list(APPEND JS_SRCS
     src/methodjit/Compiler.cpp
     src/methodjit/FastArithmetic.cpp
     src/methodjit/FastBuiltins.cpp
     src/methodjit/FastOps.cpp
     src/methodjit/FrameState.cpp
     src/methodjit/ImmutableSync.cpp
     src/methodjit/InvokeHelpers.cpp
     src/methodjit/LoopState.cpp
     src/methodjit/MethodJIT.cpp
     src/methodjit/MonoIC.cpp
     src/methodjit/PolyIC.cpp
     src/methodjit/Retcon.cpp
     src/methodjit/StubCalls.cpp
     src/methodjit/StubCompiler.cpp
     src/methodjit/TrampolineCompiler.cpp)

  if(ENABLE_ION)
    list(APPEND JS_SRCS
      src/ion/shared/BaselineCompiler-shared.cpp
      src/ion/shared/CodeGenerator-shared.cpp
      src/ion/shared/Lowering-shared.cpp
      # TODO: Platform specific sources

      src/ion/AliasAnalysis.cpp
      src/ion/AsmJS.cpp
      src/ion/AsmJSLink.cpp
      src/ion/ASMJSSignalHandlers.cpp
      src/ion/BacktracingAllocator.cpp
      src/ion/Bailouts.cpp
      src/ion/BaselineBailouts.cpp
      src/ion/BaselineCompiler.cpp
      src/ion/BaselineFrame.cpp
      src/ion/BaselineFrameInfo.cpp
      src/ion/BaselineIC.cpp
      src/ion/BaselineInspector.cpp
      src/ion/BaselineJIT.cpp
      src/ion/BitSet.cpp
      src/ion/C1Spewer.cpp
      src/ion/CodeGenerator.cpp
      src/ion/EdgeCaseAnalysis.cpp
      src/ion/EffectiveAddressAnalysis.cpp
      src/ion/Ion.cpp
      src/ion/IonAnalysis.cpp
      src/ion/IonBuilder.cpp
      src/ion/IonCaches.cpp
      src/ion/IonFrames.cpp
      src/ion/IonMacroAssembler.cpp
      src/ion/IonSpewer.cpp
      src/ion/JSONSpewer.cpp
      src/ion/LICM.cpp
      src/ion/LinearScan.cpp
      src/ion/LIR.cpp
      src/ion/LiveRangeAllocator.cpp
      src/ion/Lowering.cpp
      src/ion/MCallOptimize.cpp
      src/ion/MIR.cpp
      src/ion/MIRGraph.cpp
      src/ion/MoveResolver.cpp
      src/ion/ParallelArrayAnalysis.cpp
      src/ion/ParallelFunctions.cpp
      src/ion/RangeAnalysis.cpp
      src/ion/RegisterAllocator.cpp
      src/ion/Safepoints.cpp
      src/ion/Snapshots.cpp
      src/ion/StupidAllocator.cpp
      src/ion/TypeOracle.cpp
      src/ion/TypePolicy.cpp
      src/ion/UnreachableCodeElimination.cpp
      src/ion/ValueNumbering.cpp
      src/ion/VMFunctions.cpp)
  endif()

  if(ENABLE_ION)
    list(APPEND JS_SRCS
      src/ion/shared/Assembler-x86-shared.cpp
      src/ion/shared/BaselineCompiler-x86-shared.cpp
      src/ion/shared/BaselineIC-x86-shared.cpp
      src/ion/shared/CodeGenerator-x86-shared.cpp
      src/ion/shared/IonFrames-x86-shared.cpp
      src/ion/shared/Lowering-x86-shared.cpp
      src/ion/shared/MoveEmitter-x86-shared.cpp)
  endif()

  if(PLATFORM_ARCH == 64)
    if(WIN32)
      list(APPEND JS_SRCS
        src/methodjit/TrampolineMasmX64.asm)
    endif()
    
    if(ENABLE_ION)
      list(APPEND JS_SRCS
        src/ion/x64/Assembler-x64.cpp
        src/ion/x64/BaselineCompiler-x64.cpp
        src/ion/x64/BaselineIC-x64.cpp
        src/ion/x64/CodeGenerator-x64.cpp
        src/ion/x64/IonFrames-x64.cpp
        src/ion/x64/Lowering-x64.cpp
        src/ion/x64/MoveEmitter-x64.cpp)
    endif()
  else()
    if(ENABLE_ION)
      list(APPEND JS_SRCS
        src/ion/x86/Assembler-x86.cpp
        src/ion/x86/BaselineCompiler-x86.cpp
        src/ion/x86/BaselineIC-x86.cpp
        src/ion/x86/CodeGenerator-x86.cpp
        src/ion/x86/IonFrames-x86.cpp
        src/ion/x86/Lowering-x86.cpp
        src/ion/x86/MoveEmitter-x86.cpp)
    endif()
  endif()
endif()

list(APPEND JS_SRCS
  src/assembler/jit/ExecutableAllocator.cpp

  src/yarr/PageBlock.cpp
  src/yarr/YarrCanonicalizeUCS2.cpp
  src/yarr/YarrInterpreter.cpp
  src/yarr/YarrPattern.cpp
  src/yarr/YarrSyntaxChecker.cpp)

if(ENABLE_METHODJIT_SPEW)
  list(APPEND JS_SRCS
    src/methodjit/Logging.cpp)
endif()

if(WIN32)
  list(APPEND JS_SRCS
    src/assembler/jit/ExecutableAllocatorWin.cpp
    src/yarr/OSAllocatorWin.cpp)
endif()

if(ENABLE_METHODJIT OR ENABLE_ION OR ENABLE_YARR_JIT)
  list(APPEND JS_SRCS
    src/assembler/assembler/ASMAssembler.cpp
    src/assembler/assembler/MacroAssemblerASM.cpp
    src/assembler/assembler/MacroAssemblerX86Common.cpp)

  if(ENABLE_YARR_JIT)
  list(APPEND JS_SRCS
    src/yarr/YarrJIT.cpp)
  endif()
endif()

if(JS_HAS_CTYPES)
  list(APPEND JS_SRCS
    src/ctypes/CTypes.cpp
    src/ctypes/Library.cpp)
endif()
## End source files
###############################################################################
## Begin static library
add_library(js ${JS_SRCS})

add_dependencies(js host_jsoplengen)

target_include_directories(js
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/assembler
    ${CMAKE_CURRENT_SOURCE_DIR}/../intl/icu/src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/../intl/icu/src/i18n
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public
    # $:/src and BIN_DIR should be private but make public for now until
    # we can expose all the necessary public headers individually.
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(js PUBLIC mfbt)

if(MOZJS_THREADSAFE)
  target_link_libraries(js PUBLIC nspr winmm)
endif()

target_compile_options(js PRIVATE /W0)
## End static library
###############################################################################
## Begin auto-generated files
configure_file(js-config.h.in js-config.h)

add_executable(host_jskwgen src/jskwgen.cpp)

add_custom_command(
  OUTPUT jsautokw.h
  COMMAND host_jskwgen jsautokw.h
  DEPENDS host_jskwgen
  COMMENT "Generating jsautokw.h"
  VERBATIM)

set_source_files_properties(jsautokw.h PROPERTIES GENERATED TRUE)
target_sources(js PRIVATE jsautokw.h)

add_executable(host_jsoplengen src/jsoplengen.cpp)

add_custom_command(
  OUTPUT jsautooplen.h
  COMMAND host_jsoplengen jsautooplen.h
  DEPENDS host_jsoplengen
  COMMENT "Generating jsautooplen.h"
  VERBATIM)

set_source_files_properties(jsautooplen.h PROPERTIES GENERATED TRUE)
target_sources(js PRIVATE jsautooplen.h)

set(SELFHOSTING_SRCS
  src/builtin/Utilities.js
  src/builtin/Array.js
  src/builtin/Date.js
  src/builtin/Intl.js
  src/builtin/IntlData.js
  src/builtin/Number.js
  src/builtin/ParallelArray.js
  src/builtin/String.js)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/selfhosted.out.h
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  # HINT: /EP flag is required for MSVC C++ compiler to work
  COMMAND py src/builtin/embedjs.py -DDEBUG -p "${CMAKE_CXX_COMPILER} /EP" -m src/js.msg -o ${CMAKE_CURRENT_BINARY_DIR}/selfhosted.out.h -s ${CMAKE_CURRENT_BINARY_DIR}/selfhosted.js ${SELFHOSTING_SRCS}
  DEPENDS ${SELFHOSTING_SRCS} src/js.msg
  COMMENT "Generating selfhosted.out.h"
  VERBATIM)

set_source_files_properties(selfhosted.out.h PROPERTIES GENERATED TRUE)
target_sources(js PRIVATE selfhosted.out.h)
## End auto-generated files
###############################################################################
## Begin install directives
install(TARGETS js
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/public/js"
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

set(JS_PUBLIC_HDRS
  ${CMAKE_CURRENT_BINARY_DIR}/js-config.h
  src/js.msg
  src/jsapi.h
  src/jsalloc.h
  src/jsclass.h
  src/jsclist.h
  src/jscpucfg.h
  src/jsdbgapi.h
  src/jsdhash.h
  src/jsfriendapi.h
  src/jslock.h
  src/json.h
  src/jsproxy.h
  src/jsprf.h
  src/jsprototypes.h
  src/jsprvtd.h
  src/jspubtd.h
  src/jstypes.h
  src/jstypes.h
  src/jsutil.h
  src/jsutil.h
  src/jsversion.h
  src/jswrapper.h)

install(FILES ${JS_PUBLIC_HDRS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
## End install directives
###############################################################################
