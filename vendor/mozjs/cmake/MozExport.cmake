# Usage: MOZ_EXPORT(path FILES files)
# Example: MOZ_EXPORT(/mozilla FILES "Assertions.h")
function (MOZ_EXPORT ${_DIR})
  cmake_parse_arguments(PARSE_ARGV 1 export "" "" "FILES")
  foreach(_header ${export_FILES})
    get_filename_component(_header_file ${_header} NAME)
    make_directory(${CMAKE_BINARY_DIR}/dist${ARGV0})
    configure_file(${_header} ${CMAKE_BINARY_DIR}/dist${ARGV0}/${_header_file} COPYONLY)
  endforeach()
endfunction()
