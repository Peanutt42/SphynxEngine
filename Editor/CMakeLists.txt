file(GLOB_RECURSE EDITOR_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(Editor ${EDITOR_SOURCES})

target_include_directories(Editor PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${ENGINE_INCLUDE_DIR}
		${GLM_INCLUDE_DIR}
		${ASSIMP_INCLUDE_DIR}
		${SHADERC_INCLUDE_DIR}
)

target_compile_features(Editor PRIVATE cxx_std_20)

add_dependencies(Editor Engine)

target_link_libraries(Editor PRIVATE
		Engine
		assimp
		shaderc
)

target_precompile_headers(Editor PUBLIC src/pch.hpp)

copy_resources_folder(Editor ${CMAKE_CURRENT_BINARY_DIR})

set(EDITOR_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)