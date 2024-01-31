cmake_minimum_required(VERSION 3.24)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(d2bs)

add_subdirectory(vendor)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug build" FORCE)
endif()

set(D2BS_SRCS
  "Map/Diablo_II/ActMap.cpp"
  "Map/Diablo_II/LevelMap.cpp"
  "Map/Diablo_II/Points.cpp"

  "AutoRoot.cpp"
  "CommandLine.cpp"
  "Console.cpp"
  "Control.cpp"
  "Core.cpp"
  "D2BS.cpp"
  "D2Handlers.cpp"
  "D2Helpers.cpp"
  "D2Intercepts.cpp"
  "D2NetHandlers.cpp"
  "dde.cpp"
  "Events.cpp"
  "File.cpp"
  "Game.cpp"
  "Hash.cpp"
  "Helpers.cpp"
  "js32.cpp"
  "JSArea.cpp"
  "JSControl.cpp"
  "JSCore.cpp"
  "JSDirectory.cpp"
  "JSExits.cpp"
  "JSFile.cpp"
  "JSFileTools.cpp"
  "JSGame.cpp"
  "JSGlobalClasses.cpp"
  "JSHash.cpp"
  "JSMenu.cpp"
  "JSParty.cpp"
  "JSPresetUnit.cpp"
  "JSProfile.cpp"
  "JSRoom.cpp"
  "JSSandbox.cpp"
  "JSScreenHook.cpp"
  "JSScript.cpp"
  "JSSocket.cpp"
  "JSSQLite.cpp"
  "JSUnit.cpp"
  "MPQStats.cpp"
  "Offset.cpp"
  "Profile.cpp"
  "Room.cpp"
  "ScreenHook.cpp"
  "Script.cpp"
  "ScriptEngine.cpp"
  "StackWalker.cpp"
  "stringhash.cpp"
  "sqlite3.c"
  "Unit.cpp")

add_library(D2BS SHARED ${D2BS_SRCS})
target_include_directories(D2BS PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(D2BS PRIVATE
  WIN32_LEAN_AND_MEAN
  XP_WIN
  STATIC_JS_API)

target_link_libraries(D2BS PRIVATE
  dbghelp
  shlwapi

  js)

install(TARGETS D2BS RUNTIME DESTINATION bin/d2bs)