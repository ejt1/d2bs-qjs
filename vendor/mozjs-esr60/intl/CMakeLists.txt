add_library(intl INTERFACE)
add_library(mozjs::intl ALIAS intl)

target_include_directories(intl INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/icu/source/common
  ${CMAKE_CURRENT_SOURCE_DIR}/icu/source/i18n)
