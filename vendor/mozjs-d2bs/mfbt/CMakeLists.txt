set(MFBT_SRCS
  src/double-conversion/bignum.cc
  src/double-conversion/bignum-dtoa.cc
  src/double-conversion/cached-powers.cc
  src/double-conversion/diy-fp.cc
  src/double-conversion/double-conversion.cc
  src/double-conversion/fast-dtoa.cc
  src/double-conversion/fixed-dtoa.cc
  src/double-conversion/strtod.cc

  src/mozilla/HashFunctions.cpp
  src/mozilla/SHA1.cpp)

add_library(mfbt ${MFBT_SRCS})

target_include_directories(mfbt
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/double-conversion
    ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_definitions(mfbt PUBLIC -DIMPL_MFBT)

target_compile_options(mfbt PRIVATE /W0)

install(TARGETS mfbt
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

set(MFBT_PUBLIC_HDRS
  src/mozilla/Assertions.h
  src/mozilla/Attributes.h
  src/mozilla/BloomFilter.h
  src/mozilla/Char16.h
  src/mozilla/CheckedInt.h
  src/mozilla/Compiler.h
  src/mozilla/Constants.h
  src/mozilla/DebugOnly.h
  src/mozilla/EnumSet.h
  src/mozilla/FloatingPoint.h
  src/mozilla/GuardObjects.h
  src/mozilla/HashFunctions.h
  src/mozilla/Likely.h
  src/mozilla/LinkedList.h
  src/mozilla/MathAlgorithms.h
  src/mozilla/MemoryChecking.h
  src/mozilla/MSStdInt.h
  src/mozilla/NullPtr.h
  src/mozilla/Range.h
  src/mozilla/RangedPtr.h
  src/mozilla/RefPtr.h
  src/mozilla/Scoped.h
  src/mozilla/SHA1.h
  src/mozilla/SplayTree.h
  src/mozilla/StandardInteger.h
  src/mozilla/ThreadLocal.h
  src/mozilla/TypedEnum.h
  src/mozilla/Types.h
  src/mozilla/TypeTraits.h
  src/mozilla/Util.h
  src/mozilla/WeakPtr.h)

install(FILES ${MFBT_PUBLIC_HDRS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/mozilla)
