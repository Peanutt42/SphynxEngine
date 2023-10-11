file(GLOB_RECURSE EDITOR_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_executable(Editor ${EDITOR_SOURCES})

target_include_directories(Editor
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE
		${IMGUI_INCLUDE_DIR}
		${Vulkan_INCLUDE_DIRS}
)

target_compile_features(Editor PRIVATE cxx_std_20)

add_dependencies(Editor Engine)

target_link_libraries(Editor PRIVATE
		Engine
		assimp
		${Vulkan_SHADERC_LIB}
)

target_precompile_headers(Editor PUBLIC pch.hpp)

copy_resources_folder(Editor ${CMAKE_CURRENT_BINARY_DIR})

set(EDITOR_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)