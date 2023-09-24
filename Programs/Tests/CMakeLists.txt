file(GLOB_RECURSE TESTS_SOURCES "*.cpp")

set(TESTS_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

# download google test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.11.0  # Use the desired version tag
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(Tests ${TESTS_SOURCES})

target_include_directories(Tests PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
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

include(GoogleTest)
gtest_discover_tests(Tests)

target_precompile_headers(Tests PUBLIC pch.hpp)

define_configuration(Tests)
define_platform(Tests)