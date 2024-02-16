#####################################################################
## libc (Library)
#####################################################################
set(LIBC_SRCS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/plvrsion.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/strlen.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/strcpy.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/strdup.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/strcase.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/strcat.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/strcmp.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/strchr.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/strpbrk.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/strstr.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/strtok.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/base64.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/plerror.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/plgetopt.c")

add_library(libc ${LIBC_SRCS})
add_library(mozjs::plc ALIAS libc)

target_include_directories(libc
  PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(libc PRIVATE mozjs::nspr)

# disable all warnings
target_compile_options(libc PRIVATE /W0)

configure_file(${MOZJS_TOP_DIR}/_pl_bld.h ${CMAKE_CURRENT_BINARY_DIR}/_pl_bld.h COPYONLY)
target_sources(libc PUBLIC ${MOZJS_TOP_DIR}/_pl_bld.h)

if (MOZJS_INSTALL)
  install(TARGETS libc
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${LIBRARY_DIR})
endif()
