cmake_minimum_required(VERSION 3.26)

project(quickjs LANGUAGES C)

option(QUICKJS_BIGNUM "Include code for BigInt/BigFloat/BigDecimal and math mode" ON)
option(QUICKJS_ONLY_LIBRARY "Only build the quickjs library without examples and tests" ON)

# fetch version into variable
file(STRINGS VERSION QJS_VERSION)

###############################################################################
## libquickjs
set(LIBQUICKJS_DEFS
    QUICKJS_VERSION="${QJS_VERSION}")

if (QUICKJS_BIGNUM)
    list(APPEND LIBQUICKJS_DEFS CONFIG_BIGNUM)
endif ()

set(LIBQUICKJS_SRCS
  cutils.c
  libregexp.c
  libunicode.c
  quickjs-libc.c
  quickjs.c)

if (QUICKJS_BIGNUM)
    list(APPEND LIBQUICKJS_SRCS libbf.c)
endif ()

add_library(libquickjs ${LIBQUICKJS_SRCS})
target_compile_definitions(libquickjs PUBLIC ${LIBQUICKJS_DEFS})
target_include_directories(libquickjs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if (NOT QUICKJS_ONLY_LIBRARY)
###############################################################################
## qjsc
add_executable(qjsc qjsc.c)
target_link_libraries(qjsc libquickjs)

###############################################################################
## qjs
add_executable(qjs qjs.c repl.c)
target_link_libraries(qjs libquickjs)

# depend on qjsc for generating sources from js
add_dependencies(qjs qjsc)

# repl.c
add_custom_command(
    OUTPUT repl.c
    COMMAND qjsc -c -o repl.c -m ${CMAKE_CURRENT_SOURCE_DIR}/repl.js)

# qjscalc.c
if (QUICKJS_BIGNUM)
    # qjscalc.c
    add_custom_command(
        OUTPUT qjscalc.c
        COMMAND qjsc -fbignum -c -o qjscalc.c ${CMAKE_CURRENT_SOURCE_DIR}/qjscalc.js)
    target_sources(qjs PRIVATE qjscalc.c)
endif ()

###############################################################################
# examples

# example of static JS compilation
set(HELLO_OPTS
    -fno-string-normalize
    -fno-map
    -fno-promise
    -fno-typedarray
    -fno-regexp
    -fno-json
    -fno-eval
    -fno-proxy
    -fno-date
    -fno-module-loader)

add_custom_command(
    OUTPUT hello.c
    COMMAND qjsc -e ${HELLO_OPTS} -o hello.c ${CMAKE_CURRENT_SOURCE_DIR}/examples/hello.js)
add_executable(hello hello.c)
target_link_libraries(hello libquickjs)

# example of static JS compilation with modules
set(HELLO_MODULE_OPTS
    -fno-string-normalize
    -fno-map
    -fno-promise
    -fno-typedarray
    -fno-regexp
    -fno-json
    -fno-eval
    -fno-proxy
    -fno-date
    -m)

add_custom_command(
    OUTPUT hello_module.c
    COMMAND qjsc -e ${HELLO_OPTS} -o hello_module.c ${CMAKE_CURRENT_SOURCE_DIR}/examples/hello_module.js)
add_executable(hello_module hello_module.c)
target_link_libraries(hello_module libquickjs)

# use of an external C module (static compilation)
add_custom_command(
    OUTPUT test_fib.c
    COMMAND qjsc -e -M fib.dll,fib -m -o test_fib.c ${CMAKE_CURRENT_SOURCE_DIR}/examples/test_fib.js)
add_library(test_fib test_fib.c)
target_link_libraries(test_fib PUBLIC libquickjs)

add_library(fib SHARED examples/fib.c)
target_compile_definitions(fib PRIVATE JS_SHARED_LIBRARY)
target_link_libraries(fib PRIVATE test_fib)

###############################################################################
## tests
add_library(bjson SHARED tests/bjson.c)
target_compile_definitions(bjson PRIVATE JS_SHARED_LIBRARY)
target_link_libraries(bjson PRIVATE libquickjs)

add_library(point SHARED examples/point.c)
target_link_libraries(point PRIVATE libquickjs)

enable_testing()
add_test(NAME test_closure COMMAND qjs ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_closure.js)
add_test(NAME test_language COMMAND qjs ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_language.js)
add_test(NAME test_builtin COMMAND qjs ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_builtin.js)
add_test(NAME test_loop COMMAND qjs ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_loop.js)
add_test(NAME test_std COMMAND qjs ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_std.js)
# XXX: os.Worker uses POSIX threads, not planned to migrate to windows yet
#add_test(NAME test_worker COMMAND qjs ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_worker.js)
if (QUICKJS_BIGNUM)
    add_test(NAME test_bjson COMMAND qjs --bignum ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_bjson.js)
    add_test(NAME test_op_overloading COMMAND qjs --bignum ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_op_overloading.js)
    add_test(NAME test_bignum COMMAND qjs --bignum ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_bignum.js)
    add_test(NAME test_qjscalc COMMAND qjs --qjscalc ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_qjscalc.js)
else ()
    add_test(NAME test_bjson COMMAND qjs ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_bjson.js)
endif ()
add_test(NAME test_point COMMAND qjs ${CMAKE_CURRENT_SOURCE_DIR}/examples/test_point.js)

endif (NOT QUICKJS_ONLY_LIBRARY)
