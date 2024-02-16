#set(MOZFALLIBLE_SRCS
#  "${CMAKE_CURRENT_SOURCE_DIR}/fallible.cpp")

add_library(fallible INTERFACE ${MOZFALLIBLE_SRCS})
add_library(mozjs::fallible ALIAS fallible)

target_include_directories(fallible
#  PRIVATE
#    "${CMAKE_CURRENT_SOURCE_DIR}"
  INTERFACE
    "${MOZJS_EXPORT_PREFIX}")

if (MOZJS_INSTALL)
  install(TARGETS fallible
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${LIBRARY_DIR})
endif()

set(MOZFALLIBLE_PUBLIC_HDRS
  "${CMAKE_CURRENT_SOURCE_DIR}/fallible.h")

foreach(_header ${MOZFALLIBLE_PUBLIC_HDRS})
  get_filename_component(_header_file ${_header} NAME)
  configure_file(${_header} ${MOZJS_EXPORT_PREFIX}/mozilla/${_header_file} COPYONLY)
endforeach()
