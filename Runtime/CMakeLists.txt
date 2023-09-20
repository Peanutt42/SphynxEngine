file(GLOB_RECURSE RUNTIME_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(Runtime ${RUNTIME_SOURCES})

target_include_directories(Runtime PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(Runtime PRIVATE cxx_std_20)

add_dependencies(Runtime Engine)

target_link_libraries(Runtime PRIVATE Engine)

target_precompile_headers(Runtime PUBLIC src/pch.hpp)

copy_resources_folder(Runtime ${CMAKE_CURRENT_BINARY_DIR})

define_configuration(Runtime)
define_platform(Runtime)

set(RUNTIME_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)