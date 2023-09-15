cmake_minimum_required(VERSION 3.26)

set(VENDOR_NO_PARENT TRUE)
set(VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../vendor")
include(../vendor/CMakeLists.txt)
include(../CMakeMacros.cmake)

project(TestProject)

file(GLOB_RECURSE TESTPROJECT_SOURCES "Source/*.cpp")
file(GLOB_RECURSE TESTPROJECT_HEADERS "Source/*.hpp")
set(GENERATED_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Source/generated/Module.cpp")

add_library(TestProject SHARED ${GENERATED_MODULE_PATH} ${TESTPROJECT_SOURCES} ${TESTPROJECT_HEADERS})

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
		OUTPUT ${GENERATED_MODULE_PATH}
		# ../ before Source because the working dir is in Binaries
		COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/../Programs/ReflectionGenerator/bin/ReflectionGenerator.exe" ../Source/TestProject ../Source/generated ../../..
		COMMENT "Running ReflectionGenerator"
)

#file(WRITE ${GENERATED_MODULE_PATH} "To be generated!")

define_configuration(TestProject)
define_platform(TestProject)