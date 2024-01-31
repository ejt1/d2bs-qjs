cmake_minimum_required(VERSION 3.23)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

string(REGEX REPLACE "/W[0-4] " "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REGEX REPLACE "/W[0-4]$" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REGEX REPLACE "/W[0-4] " "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
string(REGEX REPLACE "/W[0-4]$" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# TODO: Make it an option to chose /MD or /MT
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

option(MOZJS_BUILD_TESTS "Build tests for mozjs" OFF)
option(MOZJS_THREADSAFE "Enable thread-safe support" ON)

add_definitions(-DWIN32)
add_definitions(-D_WIN32_WINNT=0x601)
add_definitions(-D_WINVER=0x601)

project(mozjs)

include(ConfigurePlatform)

add_subdirectory(js)
add_subdirectory(mfbt)
add_subdirectory(nspr)

if(MOZJS_BUILD_TESTS)
  enable_testing()
  include(PrepareTest)
  add_subdirectory(tests)
endif()
