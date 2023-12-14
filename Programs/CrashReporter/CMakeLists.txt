file(GLOB_RECURSE CRASHREPORTER_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_executable(CrashReporter ${CRASHREPORTER_SOURCES})

target_include_directories(CrashReporter PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		${GLAD_INCLUDE_DIR}
)

target_compile_features(CrashReporter PRIVATE cxx_std_20)

add_dependencies(CrashReporter
		glfw
		imgui
		yaml-cpp
		glad
)


cmake_policy(SET CMP0072 NEW)
find_package(OpenGL REQUIRED)

target_link_libraries(CrashReporter PRIVATE
		glfw
		imgui
		yaml-cpp
		glad
		${OPENGL_opengl_LIBRARY}
)

copy_file(CrashReporter $<TARGET_FILE:CrashReporter> $<TARGET_FILE_DIR:SphynxEditor>)
copy_file(CrashReporter $<TARGET_FILE:CrashReporter> $<TARGET_FILE_DIR:SphynxRuntime>)
copy_file(CrashReporter $<TARGET_FILE:CrashReporter> $<TARGET_FILE_DIR:Tests>)
add_dependencies(SphynxEditor CrashReporter)
add_dependencies(SphynxRuntime CrashReporter)
add_dependencies(Tests CrashReporter)

define_configuration(CrashReporter)
define_platform(CrashReporter)