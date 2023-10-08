file(GLOB REFLECTION_GENERATOR_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_executable(ReflectionGenerator ${REFLECTION_GENERATOR_SOURCES})

target_link_libraries(ReflectionGenerator
	yaml
)

target_compile_features(ReflectionGenerator PRIVATE cxx_std_20)

set_target_properties(ReflectionGenerator
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)