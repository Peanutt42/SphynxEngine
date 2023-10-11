cmake_minimum_required(VERSION 3.26)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(CMAKE_MSVC_RUNTIME_LIBRARY  "MultiThreadedDebugDLL")
else()
	set(CMAKE_MSVC_RUNTIME_LIBRARY  "MultiThreadedDLL")
endif()

include("cmake/SphynxMacros.cmake")

include(GNUInstallDirs)

project(SphynxEngine)

set(CMAKE_CONFIGURATION_TYPES
		"Debug"
		"Release"
		"Dist"
		CACHE STRING "Build Configurations" FORCE)

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
set_targets_folder("vendor" yaml stb_image imgui glfw uninstall update_mappings gtest tracy glad OpenAL alcommon al-excommon assimp UpdateAssimpLibsDebugSymbolsAndDLLs zlibstatic sndfile)

add_subdirectory(SphynxEngine)
add_subdirectory(SphynxEditor)
add_subdirectory(SphynxRuntime)
set_targets_folder("Core" SphynxEngine SphynxEditor SphynxRuntime)

add_subdirectory(Programs/Tests)
add_subdirectory(Programs/CrashReporter)
add_subdirectory(Programs/ReflectionGenerator)
set_targets_folder("Programs" Tests CrashReporter ReflectionGenerator)