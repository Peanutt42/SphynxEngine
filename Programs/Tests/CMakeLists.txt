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

define_configuration(CrashReporter)
define_platform(CrashReporter)