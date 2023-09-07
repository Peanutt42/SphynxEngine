cmake_minimum_required(VERSION 3.26)

project(SphynxEngine)

set(CMAKE_CONFIGURATION_TYPES
		"Debug"
		"Release"
		"Dist"
		CACHE STRING "Build Configurations" FORCE)

set(SE_RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
macro(copy_resources_folder _TARGET BIN_DIR)
	add_custom_command(
			TARGET ${_TARGET}
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${SE_RESOURCE_DIR} "${BIN_DIR}/Resources"
			COMMENT "Copying ${SE_RESOURCE_DIR} to ${BIN_DIR}/Resources"
	)
endmacro()

macro(copy_file _TARGET SOURCE DEST)
	add_custom_command(
			TARGET ${_TARGET}
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SOURCE}" "${DEST}"
			COMMENT "Copying ${SOURCE} to ${DEST}"
	)
endmacro()

macro(define_configuration _TARGET)
	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_compile_definitions(${_TARGET} PUBLIC DEBUG)
	elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
		target_compile_definitions(${_TARGET} PUBLIC RELEASE)
	elseif (CMAKE_BUILD_TYPE STREQUAL "Dist")
		target_compile_definitions(${_TARGET} PUBLIC DIST)
	else()
		message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
	endif()
endmacro()
macro(define_platform _TARGET)
	if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
		target_compile_definitions(${_TARGET} PUBLIC WINDOWS)
	elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
		target_compile_definitions(${_TARGET} PUBLIC LINUX)
	elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
		target_compile_definitions(${_TARGET} PUBLIC MACOS)
	else()
		message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
	endif()
endmacro()

add_subdirectory(vendor)
add_subdirectory(vendor/yaml)
add_subdirectory(vendor/stb_image)
add_subdirectory(vendor/imgui)
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/Tracy)
add_subdirectory(vendor/glad)
add_subdirectory(vendor/gtest)

add_subdirectory(Engine)
add_subdirectory(Editor)
add_subdirectory(EngineRuntime)

add_subdirectory(Programs/CrashReporter)
add_subdirectory(Programs/ReflectionGenerator)
add_subdirectory(Programs/Tests)

file(MAKE_DIRECTORY "bin/Debug")
file(MAKE_DIRECTORY "bin/Release")
file(MAKE_DIRECTORY "bin/Dist")