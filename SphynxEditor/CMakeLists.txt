file(GLOB_RECURSE SPHYNX_EDITOR_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_executable(SphynxEditor ${SPHYNX_EDITOR_SOURCES})

target_include_directories(SphynxEditor
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE
		${Vulkan_INCLUDE_DIRS}
)

target_compile_features(SphynxEditor PRIVATE cxx_std_20)

add_dependencies(SphynxEditor SphynxEngine)

target_link_libraries(SphynxEditor PRIVATE
		SphynxEngine
		assimp
		${Vulkan_SHADERC_LIB}
)

target_precompile_headers(SphynxEditor PUBLIC pch.hpp)

copy_content_folder(SphynxEditor ${CMAKE_CURRENT_BINARY_DIR})

set(SPHYNX_EDITOR_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)