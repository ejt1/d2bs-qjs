# disable libuv shared builds since we link statically
set(LIBUV_BUILD_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory(libuv-1.47.0)
add_subdirectory(minhook-1.3.3)

if (MOZJS_USE_PREBUILT)
  add_subdirectory(mozjs-prebuilt)
else()
  add_subdirectory(mozjs)
endif()
