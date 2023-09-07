cmake_minimum_required(VERSION 3.26)

set(VENDOR_NO_PARENT TRUE)
set(VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../vendor")
include(../vendor/CMakeLists.txt)

project(TestProject)

file(GLOB_RECURSE TESTPROJECT_SOURCES "Source/*.cpp")
file(GLOB_RECURSE TESTPROJECT_HEADERS "Source/*.hpp")

add_library(TestProject SHARED ${TESTPROJECT_SOURCES} ${TESTPROJECT_HEADERS})

target_include_directories(TestProject PUBLIC
		"Source/TestProject"
		"../TestProject/src"
		${GLM_INCLUDE_DIR}
		${TRACY_INCLUDE_DIR}
		${ENGINE_INCLUDE_DIR}
)

target_link_libraries(TestProject
		"${CMAKE_CURRENT_SOURCE_DIR}/../bin/${CMAKE_BUILD_TYPE}/Engine/Engine.lib"
		"${CMAKE_CURRENT_SOURCE_DIR}/../bin/${CMAKE_BUILD_TYPE}/vendor/Tracy/Tracy.lib"
)

target_compile_features(TestProject PRIVATE cxx_std_20)

target_compile_definitions(TestProject PRIVATE SE_GAME_MODULE)

add_custom_command(
		PRE_BUILD
		OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/Source/generated/Module.cpp"
		# ../ before Source because the working dir is in Binaries
		COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/../Programs/ReflectionGenerator/bin/ReflectionGenerator.exe" ../Source/TestProject ../Source/generated ../../..
		COMMENT "Running ReflectionGenerator"
)


if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(TestProject PUBLIC DEBUG)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
	target_compile_definitions(TestProject PUBLIC RELEASE)
elseif (CMAKE_BUILD_TYPE STREQUAL "Dist")
	target_compile_definitions(TestProject PUBLIC DIST)
else()
	message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	target_compile_definitions(TestProject PUBLIC WINDOWS)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
	target_compile_definitions(TestProject PUBLIC LINUX)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	target_compile_definitions(TestProject PUBLIC MACOS)
else()
	message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()