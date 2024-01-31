cmake_minimum_required(VERSION 3.24)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

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

  "${CMAKE_CURRENT_SOURCE_DIR}/src/Main/D2BS.cpp"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/Map/Diablo_II/ActMap.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Map/Diablo_II/LevelMap.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Map/Diablo_II/Points.cpp"
  
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Misc/CommandLine.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Misc/Console.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Misc/Events.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Misc/Helpers.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Misc/js32.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Misc/Offset.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Misc/sqlite3.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Misc/stringhash.cpp"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/C/Control.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/C/Core.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/C/dde.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/C/File.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/C/Game.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/C/Hash.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/C/MPQStats.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/C/Profile.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/C/Room.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/C/ScreenHook.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/C/Unit.cpp"
  
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
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/JS/JSSandbox.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/JS/JSScreenHook.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/JS/JSScript.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/JS/JSSocket.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/JS/JSSQLite.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Core/JS/JSUnit.cpp"
  
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Engine/AutoRoot.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Engine/Script.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Spidermonkey Engine/Engine/ScriptEngine.cpp"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/StackWalker.cpp"

  "version.rc")

add_library(D2BS SHARED ${D2BS_SRCS})
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
  WIN32_LEAN_AND_MEAN
  XP_WIN
  STATIC_JS_API)

target_link_libraries(D2BS PRIVATE
  dbghelp
  shlwapi

  js)

install(TARGETS D2BS RUNTIME DESTINATION bin/d2bs)