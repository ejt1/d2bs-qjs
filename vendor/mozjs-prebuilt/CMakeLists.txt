add_library(mozjs INTERFACE)
target_link_libraries(mozjs INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/lib/mozjs.lib"
  winmm)
target_include_directories(mozjs INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include")
