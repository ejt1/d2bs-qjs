cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(mozjs)

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
add_definitions(-DJS_DEFAULT_JITREPORT_GRANULARITY=3)
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

add_subdirectory(intl)
add_subdirectory(js)
add_subdirectory(memory)
add_subdirectory(mfbt)
add_subdirectory(mozglue)
add_subdirectory(modules)
add_subdirectory(nsprpub)
