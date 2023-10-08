cmake_minimum_required(VERSION 3.26)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(CMAKE_MSVC_RUNTIME_LIBRARY  "MultiThreadedDebugDLL")
else()
	set(CMAKE_MSVC_RUNTIME_LIBRARY  "MultiThreadedDLL")
endif()

include("vendor/Macros.cmake")

project(SphynxEngine)

set(CMAKE_CONFIGURATION_TYPES
		"Debug"
		"Release"
		"Dist"
		CACHE STRING "Build Configurations" FORCE)

set(SE_RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Resources")

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/cmake_install")

add_subdirectory(vendor)
add_subdirectory(vendor/yaml)
set_target_properties(yaml PROPERTIES FOLDER "vendor")
add_subdirectory(vendor/stb_image)
set_target_properties(stb_image PROPERTIES FOLDER "vendor")
add_subdirectory(vendor/imgui)
set_target_properties(imgui PROPERTIES FOLDER "vendor")
add_subdirectory(vendor/glfw)
set_target_properties(glfw PROPERTIES FOLDER "vendor")
add_subdirectory(vendor/gtest)
set_target_properties(gtest PROPERTIES FOLDER "vendor")
add_subdirectory(vendor/Tracy)
set_target_properties(tracy PROPERTIES FOLDER "vendor")
add_subdirectory(vendor/glad)
set_target_properties(glad PROPERTIES FOLDER "vendor")
add_subdirectory(vendor/openal-soft)
set_target_properties(OpenAL PROPERTIES FOLDER "vendor")
add_subdirectory(vendor/assimp)
set_target_properties(assimp PROPERTIES FOLDER "vendor")
add_subdirectory(vendor/libsndfile)
set_target_properties(sndfile PROPERTIES FOLDER "vendor")

add_subdirectory(Engine)
set_target_properties(Engine PROPERTIES FOLDER "Core")
add_subdirectory(Editor)
set_target_properties(Editor PROPERTIES FOLDER "Core")
add_subdirectory(Runtime)
set_target_properties(Runtime PROPERTIES FOLDER "Core")

add_subdirectory(Programs/Tests)
set_target_properties(Tests PROPERTIES FOLDER "Programs")
add_subdirectory(Programs/CrashReporter)
set_target_properties(CrashReporter PROPERTIES FOLDER "Programs")
add_subdirectory(Programs/ReflectionGenerator)
set_target_properties(ReflectionGenerator PROPERTIES FOLDER "Programs")