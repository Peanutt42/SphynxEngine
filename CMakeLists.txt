cmake_minimum_required(VERSION 3.26)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(CMAKE_MSVC_RUNTIME_LIBRARY  "MultiThreadedDebugDLL")
else()
	set(CMAKE_MSVC_RUNTIME_LIBRARY  "MultiThreadedDLL")
endif()

include(CMakeMacros.cmake)

project(SphynxEngine)

set(CMAKE_CONFIGURATION_TYPES
		"Debug"
		"Release"
		"Dist"
		CACHE STRING "Build Configurations" FORCE)

set(SE_RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Resources")

add_subdirectory(vendor)
add_subdirectory(vendor/yaml)
add_subdirectory(vendor/stb_image)
add_subdirectory(vendor/imgui)
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/Tracy)
add_subdirectory(vendor/glad)
add_subdirectory(vendor/gtest)
add_subdirectory(vendor/assimp)

add_subdirectory(Engine)
add_subdirectory(Editor)
add_subdirectory(EngineRuntime)

add_subdirectory(Programs/CrashReporter)
add_subdirectory(Programs/ReflectionGenerator)
add_subdirectory(Programs/Tests)

file(MAKE_DIRECTORY "bin/Debug")
file(MAKE_DIRECTORY "bin/Release")
file(MAKE_DIRECTORY "bin/Dist")