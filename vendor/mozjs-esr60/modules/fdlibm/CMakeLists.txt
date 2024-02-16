add_library(fdlibm
  "${CMAKE_CURRENT_SOURCE_DIR}/src/e_acos.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/e_acosh.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/e_asin.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/e_atan2.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/e_atanh.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/e_cosh.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/e_exp.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/e_hypot.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/e_log.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/e_log10.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/e_log2.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/e_pow.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/e_sinh.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/e_sqrt.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/k_exp.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/s_asinh.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/s_atan.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/s_cbrt.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/s_ceil.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/s_ceilf.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/s_copysign.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/s_expm1.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/s_fabs.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/s_floor.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/s_floorf.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/s_log1p.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/s_nearbyint.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/s_rint.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/s_rintf.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/s_scalbn.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/s_tanh.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/s_trunc.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/s_truncf.cpp")

add_library(mozjs::fdlibm ALIAS fdlibm)

target_link_libraries(fdlibm PRIVATE mfbt)

# disable all warnings
target_compile_options(fdlibm PRIVATE /W0)

set(FDLIBM_PUBLIC_HDRS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/fdlibm.h")

foreach(_header ${FDLIBM_PUBLIC_HDRS})
  get_filename_component(_header_file ${_header} NAME)
  configure_file(${_header} ${MOZJS_EXPORT_PREFIX}/${_header_file} COPYONLY)
endforeach()
