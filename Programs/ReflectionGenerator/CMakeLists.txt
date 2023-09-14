add_executable(ReflectionGenerator "src/main.cpp")

target_include_directories(ReflectionGenerator PUBLIC "src")

target_compile_features(ReflectionGenerator PRIVATE cxx_std_20)

set_target_properties(ReflectionGenerator PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)

define_configuration(ReflectionGenerator)
define_platform(ReflectionGenerator)