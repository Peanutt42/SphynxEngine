include(GNUInstallDirs)

file(GLOB_RECURSE YAML_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)
file(GLOB_RECURSE YAML_HEADERS
		${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
)

add_library(yaml STATIC ${YAML_SOURCES} ${YAML_HEADERS})

target_include_directories(yaml
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_features(yaml PRIVATE cxx_std_14)