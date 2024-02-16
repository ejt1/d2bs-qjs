#####################################################################
## mfbt (Library)
#####################################################################
set(MFBT_SRCS
  "${CMAKE_CURRENT_SOURCE_DIR}/double-conversion/bignum.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/double-conversion/bignum-dtoa.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/double-conversion/cached-powers.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/double-conversion/diy-fp.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/double-conversion/double-conversion.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/double-conversion/fast-dtoa.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/double-conversion/fixed-dtoa.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/double-conversion/strtod.cc"

  "${CMAKE_CURRENT_SOURCE_DIR}/decimal/Decimal.cpp"
  
  "${CMAKE_CURRENT_SOURCE_DIR}/Assertions.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ChaosMode.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Compression.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/FloatingPoint.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/HashFunctions.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/JSONWriter.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/lz4.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Poison.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/SHA1.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/TaggedAnonymousMemory.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/unused.cpp")

add_library(mfbt ${LIBRARY_TYPE} ${MFBT_SRCS})
add_library(mozjs::mfbt ALIAS mfbt)

target_include_directories(mfbt
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
  PUBLIC
    "${MOZJS_EXPORT_PREFIX}")

target_compile_definitions(mfbt PUBLIC -DIMPL_MFBT)

# disable all warnings
target_compile_options(mfbt PRIVATE /W0)

if (MOZJS_INSTALL)
  install(TARGETS mfbt
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${LIBRARY_DIR})
endif()

set(DOUBLE_CONVERSION_PUBLIC_HDRS
  "${CMAKE_CURRENT_SOURCE_DIR}/double-conversion/double-conversion.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/double-conversion/utils.h")

set(MFBT_PUBLIC_HDRS
  "${CMAKE_CURRENT_SOURCE_DIR}/Alignment.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/AllocPolicy.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/AlreadyAddRefed.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Array.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ArrayUtils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Assertions.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Atomics.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Attributes.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/BinarySearch.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/BloomFilter.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/BufferList.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Casting.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ChaosMode.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Char16.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/CheckedInt.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Compiler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Compression.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/DebugOnly.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/DefineEnum.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/DoublyLinkedList.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/EndianUtils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/EnumeratedArray.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/EnumeratedRange.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/EnumSet.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/EnumTypeTraits.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/FastBernoulliTrial.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/FloatingPoint.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/FStream.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/GuardObjects.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/HashFunctions.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/IndexSequence.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/IntegerPrintfMacros.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/IntegerRange.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/IntegerTypeTraits.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/JSONWriter.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Likely.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/LinkedList.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/LinuxSignal.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MacroArgs.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MacroForEach.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MathAlgorithms.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Maybe.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MaybeOneOf.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MemoryChecking.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MemoryReporting.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Move.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/NullPtr.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Opaque.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/OperatorNewExtensions.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Pair.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PodOperations.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Poison.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Range.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/RangedArray.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/RangedPtr.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ReentrancyGuard.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/RefCounted.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/RefCountType.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/RefPtr.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Result.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ResultExtensions.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ReverseIterator.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/RollingMean.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Saturate.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Scoped.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ScopeExit.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SegmentedVector.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SHA1.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SmallPointerArray.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Span.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/SplayTree.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Sprintf.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/StaticAnalysisFunctions.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TaggedAnonymousMemory.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TemplateLib.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TextUtils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ThreadLocal.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ThreadSafeWeakPtr.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/ToString.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Tuple.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TypedEnumBits.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Types.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TypeTraits.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/UniquePtr.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/UniquePtrExtensions.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/unused.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Variant.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Vector.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/WeakPtr.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/WindowsVersion.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/WrappingOperations.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/XorShift128PlusRNG.h")

foreach(_header ${MFBT_PUBLIC_HDRS})
  get_filename_component(_header_file ${_header} NAME)
  configure_file(${_header} ${MOZJS_EXPORT_PREFIX}/mozilla/${_header_file} COPYONLY)
endforeach()

foreach(_header ${DOUBLE_CONVERSION_PUBLIC_HDRS})
  get_filename_component(_header_file ${_header} NAME)
  configure_file(${_header} ${MOZJS_EXPORT_PREFIX}/double-conversion/${_header_file} COPYONLY)
endforeach()

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/decimal/Decimal.h
  ${MOZJS_EXPORT_PREFIX}/mozilla/decimal/Decimal.h
  COPYONLY)
  
if (MOZJS_INSTALL)
  install(FILES ${MFBT_PUBLIC_HDRS}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/mozilla)

  install(FILES ${DOUBLE_CONVERSION_PUBLIC_HDRS}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/double-conversion)
endif()
