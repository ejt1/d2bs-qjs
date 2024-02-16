#####################################################################
## mozglue (Library)
#####################################################################
set(MOZGLUE_SRCS
  "${CMAKE_CURRENT_SOURCE_DIR}/AutoProfilerLabel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ConditionVariable_windows.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Mutex_windows.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Printf.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/StackWalk.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/TimeStamp.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/TimeStamp_windows.cpp")

add_library(mozglue ${LIBRARY_TYPE} ${MOZGLUE_SRCS})
add_library(mozjs::mozglue ALIAS mozglue)

target_include_directories(mozglue
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
  PUBLIC
    "${MOZJS_EXPORT_PREFIX}")

target_link_libraries(mozglue PUBLIC dbghelp)

target_compile_definitions(mozglue PUBLIC -DIMPL_MFBT -DMOZ_HAS_MOZGLUE)

# disable all warnings
target_compile_options(mozglue PRIVATE /W0)

if (MOZJS_INSTALL)
  install(TARGETS mozglue
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${LIBRARY_DIR})
endif()

set(MOZGLUE_PUBLIC_HDRS
  "${CMAKE_CURRENT_SOURCE_DIR}/AutoProfilerLabel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PlatformConditionVariable.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/PlatformMutex.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Printf.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/StackWalk.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/StackWalk_windows.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TimeStamp.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TimeStamp_windows.h")

foreach(_header ${MOZGLUE_PUBLIC_HDRS})
  get_filename_component(_header_file ${_header} NAME)
  configure_file(${_header} ${MOZJS_EXPORT_PREFIX}/mozilla/${_header_file} COPYONLY)
endforeach()

if (MOZJS_INSTALL)
  install(FILES ${MFBT_PUBLIC_HDRS}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/mozilla)
endif()
