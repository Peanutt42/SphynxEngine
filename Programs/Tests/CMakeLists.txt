file(GLOB_RECURSE TESTS_SOURCES "*.cpp")

enable_testing()

add_executable(Tests ${TESTS_SOURCES})

target_include_directories(Tests PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		${SPHYNX_ENGINE_INCLUDE_DIR}
		${YAML_INCLUDE_DIR}
		${GLM_INCLUDE_DIR}
		${TRACY_INCLUDE_DIR}
		${IMGUI_INCLUDE_DIR}
)

target_compile_features(Tests PRIVATE cxx_std_20)

target_link_libraries(Tests PRIVATE
		gtest
		SphynxEngine
		yaml
		imgui
)

include(GoogleTest)
gtest_discover_tests(Tests)

target_precompile_headers(Tests PUBLIC pch.hpp)