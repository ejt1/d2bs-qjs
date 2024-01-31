message(STATUS "SYSTEM: Configure platform dependent settings")

if(CMAKE_SIZEOF_VOID_P MATCHES 8)
  set(PLATFORM_ARCH 64)
else()
  set(PLATFORM_ARCH 32)
endif()

if(WIN32)
  add_definitions(-DXP_WIN)
  add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()
