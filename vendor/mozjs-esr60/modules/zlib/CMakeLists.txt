add_library(zlib
  "${CMAKE_CURRENT_SOURCE_DIR}/src/adler32.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/compress.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/crc32.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/deflate.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/gzclose.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/gzlib.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/gzread.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/gzwrite.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/infback.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/inffast.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/inflate.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/inftrees.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/trees.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/uncompr.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/zutil.c")
add_library(mozjs::zlib ALIAS zlib)

target_link_libraries(zlib PRIVATE mfbt)

# disable all warnings
target_compile_options(zlib PRIVATE /W0)

set(ZLIB_PUBLIC_HDRS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/mozzconf.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/zconf.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/zlib.h")

foreach(_header ${ZLIB_PUBLIC_HDRS})
  get_filename_component(_header_file ${_header} NAME)
  configure_file(${_header} ${MOZJS_EXPORT_PREFIX}/${_header_file} COPYONLY)
endforeach()
