cmake_minimum_required(VERSION 3.26)

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

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake_install")

set(PACKAGE_FIND_USE_STATIC_LIBS OFF)
find_package(Engine CONFIG REQUIRED)

project(TestProject)

file(GLOB_RECURSE TEST_PROJECT_SOURCES "Source/*.cpp")
file(GLOB_RECURSE TEST_PROJECT_HEADERS "Source/*.hpp")
add_library(TestProject SHARED ${TEST_PROJECT_SOURCES} ${TEST_PROJECT_HEADERS})

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