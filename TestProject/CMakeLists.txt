cmake_minimum_required(VERSION 3.26)

set(VENDOR_NO_PARENT TRUE)
set(SPHYNX_ENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(VENDOR_DIR "${SPHYNX_ENGINE_DIR}/vendor")

# TODO: Make SphynxEngine intallable
include("../vendor/CMakeLists.txt")
include("../vendor/Macros.cmake")

project(TestProject)

file(GLOB_RECURSE TESTPROJECT_SOURCES "Source/TestProject/*.cpp")
file(GLOB_RECURSE TESTPROJECT_HEADERS "Source/TestProject/*.hpp")
set(GENERATED_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Source/generated.cpp")
set(REFLECTION_GENERATOR_BIN "${SPHYNX_ENGINE_DIR}/Programs/ReflectionGenerator/bin/ReflectionGenerator.exe")

add_library(TestProject SHARED ${GENERATED_MODULE_PATH} ${TESTPROJECT_SOURCES} ${TESTPROJECT_HEADERS} )

target_include_directories(TestProject PUBLIC
		"Source/TestProject"
		${GLM_INCLUDE_DIR}
		${TRACY_INCLUDE_DIR}
		${ENGINE_INCLUDE_DIR}
		${IMGUI_INCLUDE_DIR}
)

target_link_libraries(TestProject
		"${SPHYNX_ENGINE_DIR}/bin/${CMAKE_BUILD_TYPE}/Engine/Engine.lib"
		"${SPHYNX_ENGINE_DIR}/bin/${CMAKE_BUILD_TYPE}/vendor/Tracy/Tracy.lib"
)

target_compile_features(TestProject PRIVATE cxx_std_20)

target_compile_definitions(TestProject PUBLIC SE_GAME_MODULE)

add_custom_command(
		OUTPUT ${GENERATED_MODULE_PATH}
		COMMAND ${REFLECTION_GENERATOR_BIN} "Source/TestProject" ${GENERATED_MODULE_PATH} ../..
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

define_configuration(TestProject)
define_platform(TestProject)

string(TIMESTAMP TIMESTAMP "%Y%m%d%H%M%S" UTC)

set_target_properties(TestProject
	PROPERTIES OUTPUT_NAME "TestProject_${TIMESTAMP}"
)

add_custom_target(
	make_cmake_rerun
	COMMAND ${CMAKE_COMMAND} -U TIMESTAMP ${CMAKE_BINARY_DIR}
)
add_dependencies(TestProject make_cmake_rerun)

# Cleanup old game files
file(GLOB OLD_DLL_FILES
	"${CMAKE_CURRENT_BINARY_DIR}/*.dll"
	"${CMAKE_CURRENT_BINARY_DIR}/*.exp"
	"${CMAKE_CURRENT_BINARY_DIR}/*.ilk"
	"${CMAKE_CURRENT_BINARY_DIR}/*.lib"
)
foreach(OLD_DLL_FILE ${OLD_DLL_FILES})
	file(REMOVE ${OLD_DLL_FILE})
endforeach()