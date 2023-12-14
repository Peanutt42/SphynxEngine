cmake_minimum_required(VERSION 3.22)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

include("CMake/SphynxMacros.cmake")

enable_language(CXX C)
include(GNUInstallDirs)

project(SphynxEngine)

set(SE_CONTENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Content")

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/CMakeInstall")

add_subdirectory(ThirdParty)

add_subdirectory(SphynxEngine)
add_subdirectory(SphynxEditor)
add_subdirectory(SphynxRuntime)

add_subdirectory(Programs/Tests)
add_subdirectory(Programs/CrashReporter)
add_subdirectory(Programs/ReflectionGenerator)