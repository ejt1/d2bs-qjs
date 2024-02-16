set(MOZALLOC_SRCS
  "${CMAKE_CURRENT_SOURCE_DIR}/mozalloc.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/mozalloc_abort.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/mozalloc_oom.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/winheap.cpp")

add_library(mozalloc ${MOZALLOC_SRCS})
add_library(mozjs::mozalloc ALIAS mozalloc)

target_include_directories(mozalloc
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
  PUBLIC
    "${MOZJS_EXPORT_PREFIX}")

target_compile_definitions(mozalloc PUBLIC -DIMPL_MFBT)

# disable all warnings
target_compile_options(mozalloc PRIVATE /W0)

if (MOZJS_INSTALL)
  install(TARGETS mozalloc
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${LIBRARY_DIR})
endif()

set(MOZALLOC_PUBLIC_HDRS
  "${CMAKE_CURRENT_SOURCE_DIR}/mozalloc.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/mozalloc_abort.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/mozalloc_oom.h")

foreach(_header ${MOZALLOC_PUBLIC_HDRS})
  get_filename_component(_header_file ${_header} NAME)
  configure_file(${_header} ${MOZJS_EXPORT_PREFIX}/mozilla/${_header_file} COPYONLY)
endforeach()
