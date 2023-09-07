file(GLOB_RECURSE TESTS_SOURCES "src/*.cpp")

add_executable(Tests ${TESTS_SOURCES})

target_include_directories(Tests PUBLIC
		"src"
		${GTEST_INCLUDE_DIR}
		${ENGINE_INCLUDE_DIR}
		${YAML_INCLUDE_DIR}
		${GLM_INCLUDE_DIR}
		${TRACY_INCLUDE_DIR}
		${IMGUI_INCLUDE_DIR}
)

target_compile_features(Tests PRIVATE cxx_std_20)

target_link_libraries(Tests PRIVATE
		gtest
		Engine
		yaml
		imgui
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(CrashReporter PUBLIC DEBUG)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
	target_compile_definitions(CrashReporter PUBLIC RELEASE)
elseif (CMAKE_BUILD_TYPE STREQUAL "Dist")
	target_compile_definitions(CrashReporter PUBLIC DIST)
else()
	message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	target_compile_definitions(CrashReporter PUBLIC WINDOWS)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
	target_compile_definitions(CrashReporter PUBLIC LINUX)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	target_compile_definitions(CrashReporter PUBLIC MACOS)
else()
	message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()