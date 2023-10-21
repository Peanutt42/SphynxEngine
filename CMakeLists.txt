cmake_minimum_required(VERSION 3.22)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

include("cmake/SphynxMacros.cmake")

enable_language(CXX C)
include(GNUInstallDirs)

project(SphynxEngine)

set(SE_CONTENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Content")

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/cmake_install")

add_subdirectory(vendor)
add_subdirectory(vendor/yaml)
add_subdirectory(vendor/stb_image)
add_subdirectory(vendor/imgui)
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/gtest)
add_subdirectory(vendor/Tracy)
add_subdirectory(vendor/glad)
add_subdirectory(vendor/openal-soft)
add_subdirectory(vendor/assimp)
add_subdirectory(vendor/libsndfile)
add_subdirectory(vendor/backward-cpp)
add_subdirectory(vendor/fmt)
set_target_properties(sndfile PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(OpenAL PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(fmt PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_subdirectory(SphynxEngine)
add_subdirectory(SphynxEditor)
add_subdirectory(SphynxRuntime)

add_subdirectory(Programs/Tests)
add_subdirectory(Programs/CrashReporter)
add_subdirectory(Programs/ReflectionGenerator)