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


# install
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_SOURCE_DIR}/cmake/ReflectionGeneratorConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ReflectionGeneratorConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/ReflectionGenerator"
)

install(
	TARGETS ReflectionGenerator
	EXPORT ReflectionGeneratorTargets
	RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
	INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
	DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	FILES_MATCHING PATTERN "*.hpp"
)

install(
	FILES "${CMAKE_CURRENT_BINARY_DIR}/ReflectionGeneratorConfig.cmake"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/ReflectionGenerator"
)

install(
	EXPORT ReflectionGeneratorTargets
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/ReflectionGenerator"
)