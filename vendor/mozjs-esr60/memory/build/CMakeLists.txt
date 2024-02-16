set(MOZMEMORY_SRCS
  "${CMAKE_CURRENT_SOURCE_DIR}/fallback.cpp")

add_library(mozmemory ${MOZMEMORY_SRCS})
add_library(mozjs::mozmemory ALIAS mozmemory)

target_include_directories(mozmemory
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
  PUBLIC
    "${MOZJS_EXPORT_PREFIX}")

target_compile_definitions(mozmemory PUBLIC -DMOZ_MEMORY_IMPL -DMOZ_HAS_MOZGLUE)

# disable all warnings
target_compile_options(mozmemory PRIVATE /W0)

if (MOZJS_INSTALL)
  install(TARGETS mozmemory
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${LIBRARY_DIR})
endif()

set(MOZMEMORY_PUBLIC_HDRS
  "${CMAKE_CURRENT_SOURCE_DIR}/malloc_decls.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/mozjemalloc_types.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/mozmemory.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/mozmemory_wrap.h")

foreach(_header ${MOZMEMORY_PUBLIC_HDRS})
  get_filename_component(_header_file ${_header} NAME)
  configure_file(${_header} ${MOZJS_EXPORT_PREFIX}/${_header_file} COPYONLY)
endforeach()
