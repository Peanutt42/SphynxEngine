file(GLOB_RECURSE ENGINERUNTIME_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(EngineRuntime ${ENGINERUNTIME_SOURCES})

target_include_directories(EngineRuntime PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${ENGINE_INCLUDE_DIR}
		${GLM_INCLUDE_DIR}
)

target_compile_features(EngineRuntime PRIVATE cxx_std_20)

add_dependencies(EngineRuntime Engine)

target_link_libraries(EngineRuntime PRIVATE Engine)

target_precompile_headers(EngineRuntime PUBLIC src/pch.hpp)

copy_resources_folder(EngineRuntime ${CMAKE_CURRENT_BINARY_DIR})

define_configuration(EngineRuntime)
define_platform(EngineRuntime)

set(ENGINERUNTIME_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)