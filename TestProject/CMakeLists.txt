cmake_minimum_required(VERSION 3.26)

project(TestProject)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(CMAKE_MSVC_RUNTIME_LIBRARY  "MultiThreadedDebugDLL")
else()
	set(CMAKE_MSVC_RUNTIME_LIBRARY  "MultiThreadedDLL")
endif()

set(CMAKE_CONFIGURATION_TYPES
		"Debug"
		"Release"
		"Dist"
		CACHE STRING "Build Configurations" FORCE)

set(SE_CMAKE_INSTALL "${CMAKE_CURRENT_SOURCE_DIR}/../cmake_install")
list(APPEND CMAKE_PREFIX_PATH ${SE_CMAKE_INSTALL})

find_package(SphynxEngine CONFIG REQUIRED)
find_package(ReflectionGenerator CONFIG REQUIRED)

file(GLOB_RECURSE TEST_PROJECT_SOURCES "Source/*.cpp")
file(GLOB_RECURSE TEST_PROJECT_HEADERS "Source/*.hpp")
set(GENERATED_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Source/generated/generated.cpp")
set(REFLECTION_GENERATOR_BIN "${SE_CMAKE_INSTALL}/bin/ReflectionGenerator.exe")
add_library(TestProject SHARED ${TEST_PROJECT_SOURCES} ${TEST_PROJECT_HEADERS} ${GENERATED_MODULE_PATH})

set(TEST_PROJECT_BINARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Binaries")

set_target_properties(TestProject PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY ${TEST_PROJECT_BINARY_DIR}
	LIBRARY_OUTPUT_DIRECTORY ${TEST_PROJECT_BINARY_DIR}
	RUNTIME_OUTPUT_DIRECTORY ${TEST_PROJECT_BINARY_DIR}
)

target_include_directories(TestProject PRIVATE
	${Engine_INCLUDE_DIRS}
)

target_link_libraries(TestProject PRIVATE
	${Engine_LIBRARIES}
	tracy
)

target_precompile_headers(TestProject PUBLIC "Source/pch.hpp")

target_compile_features(TestProject PRIVATE cxx_std_20)

target_compile_definitions(TestProject PRIVATE SE_GAME)

define_configuration(TestProject)
define_platform(TestProject)


add_custom_command(
		OUTPUT ${GENERATED_MODULE_PATH}
		COMMAND ${REFLECTION_GENERATOR_BIN} "${CMAKE_CURRENT_SOURCE_DIR}/Source/TestProject/" "${CMAKE_CURRENT_SOURCE_DIR}/Source/generated"
		DEPENDS ${REFLECTION_GENERATOR_BIN}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMENT "Running ReflectionGenerator"
)

# removes generated file
add_custom_command(
		TARGET TestProject
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E remove ${GENERATED_MODULE_PATH}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# creates "BuildFinished" file as a trigger for reload
add_custom_command(
		TARGET TestProject
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E touch "BuildFinished"
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_command(
		TARGET TestProject
		PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E remove "BuildFinished"
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

string(TIMESTAMP TIMESTAMP "%Y%m%d%H%M%S" UTC)

set_target_properties(TestProject
	PROPERTIES COMPILE_PDB_NAME "TestProject_${TIMESTAMP}"
)

add_custom_target(
	make_cmake_rerun
	COMMAND ${CMAKE_COMMAND} -U TIMESTAMP ${CMAKE_BINARY_DIR}
)
add_dependencies(TestProject make_cmake_rerun)

# Cleanup old game files
file(GLOB OLD_DLL_FILES
	"${CMAKE_CURRENT_BINARY_DIR}/*.dll"
	"${CMAKE_CURRENT_BINARY_DIR}/*.pdb"
	"${CMAKE_CURRENT_BINARY_DIR}/*.exp"
	"${CMAKE_CURRENT_BINARY_DIR}/*.ilk"
	"${CMAKE_CURRENT_BINARY_DIR}/*.lib"
)
foreach(OLD_DLL_FILE ${OLD_DLL_FILES})
	file(REMOVE ${OLD_DLL_FILE})
endforeach()