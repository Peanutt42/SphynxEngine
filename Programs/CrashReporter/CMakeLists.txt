file(GLOB_RECURSE CRASHREPORTER_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(CrashReporter ${CRASHREPORTER_SOURCES})

target_include_directories(CrashReporter PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${IMGUI_INCLUDE_DIR}
		${GLFW_INCLUDE_DIR}
		${YAML_INCLUDE_DIR}
		${GLAD_INCLUDE_DIR}
)

target_compile_features(CrashReporter PRIVATE cxx_std_20)

add_dependencies(CrashReporter
		glfw
		imgui
		yaml
		glad
)

target_link_libraries(CrashReporter PRIVATE
		glfw
		imgui
		yaml
		glad
		${OPENGL_LIB}
)

copy_file(CrashReporter "$<TARGET_FILE:CrashReporter>" ${EDITOR_BIN_DIR})
copy_file(CrashReporter "$<TARGET_FILE:CrashReporter>" ${ENGINERUNTIME_BIN_DIR})
add_dependencies(Editor CrashReporter)
add_dependencies(EngineRuntime CrashReporter)

define_configuration(CrashReporter)
define_platform(CrashReporter)