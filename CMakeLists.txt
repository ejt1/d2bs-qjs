cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# remove any /W compile flags
string(REGEX REPLACE "/W[0-4] " "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REGEX REPLACE "/W[0-4]$" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REGEX REPLACE "/W[0-4] " "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
string(REGEX REPLACE "/W[0-4]$" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if (NOT D2BS_VERSION)
  set(D2BS_VERSION "local" CACHE STRING "")
endif()

project(d2bs)

add_subdirectory(vendor)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug build" FORCE)
endif()

set(D2BS_SRCS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Diablo II/Handlers/D2Handlers.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Diablo II/Handlers/D2NetHandlers.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Diablo II/Patches/D2Intercepts.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Diablo II/D2Helpers.cpp"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/Game/Drlg/D2DrlgDrlg.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Game/UI/NPCMenu.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Game/Units/Units.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Game/D2Automap.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Game/D2Gfx.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Game/D2Skills.cc"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/Map/Diablo_II/ActMap.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Map/Diablo_II/LevelMap.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Map/Diablo_II/Points.cpp"
  
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Misc/CommandLine.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Misc/Console.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Misc/Events.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Misc/Helpers.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Misc/Localization.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Misc/Offset.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Misc/stringhash.cpp"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/C/Control.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/C/Core.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/C/File.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/C/Game.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/C/Hash.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/C/MPQStats.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/C/Profile.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/C/ScreenHook.cpp"
  
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/JS/JSArea.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/JS/JSControl.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/JS/JSCore.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/JS/JSDirectory.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/JS/JSExits.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/JS/JSFile.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/JS/JSFileTools.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/JS/JSGame.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/JS/JSGlobalClasses.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/JS/JSHash.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/JS/JSMenu.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/JS/JSParty.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/JS/JSPresetUnit.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/JS/JSProfile.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/JS/JSRoom.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/JS/JSScreenHook.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/JS/JSScript.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/JS/JSSocket.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/JS/JSUnit.cpp"
  
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Engine/Bindings.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Engine/js32.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Engine/Script.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Engine/ScriptEngine.cpp"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/Engine.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Globals.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc"

  "version.rc")

add_library(D2BS SHARED ${D2BS_SRCS})

target_compile_options(D2BS PRIVATE /W4)

target_include_directories(D2BS PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/Diablo II"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/Diablo II/Handlers"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/Diablo II/Patches"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/Main"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/Map"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/Map/Diablo_II"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/Map/Path"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/Map/Path/Reduction"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/Misc"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/SpiderMonkey Engine/Core"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/SpiderMonkey Engine/Core/API"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/SpiderMonkey Engine/Core/C"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/SpiderMonkey Engine/Core/JS"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/SpiderMonkey Engine/Engine")

target_compile_definitions(D2BS PRIVATE
  WIN32
  WIN32_LEAN_AND_MEAN
  XP_WIN
  STATIC_JS_API
  D2BS_VERSION="${D2BS_VERSION}")

target_link_libraries(D2BS PRIVATE
  # windows dependencies
  dbghelp
  shlwapi

  # vendor dependencies
  libquickjs
  minhook
  # libuv
  uv_a)

install(TARGETS D2BS RUNTIME DESTINATION bin/d2bs)
