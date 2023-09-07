file(GLOB_RECURSE EDITOR_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(Editor ${EDITOR_SOURCES})

target_include_directories(Editor PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${ENGINE_INCLUDE_DIR}
		${GLM_INCLUDE_DIR}
		${ASSIMP_INCLUDE_DIR}
)

target_compile_features(Editor PRIVATE cxx_std_20)

add_dependencies(Editor Engine)

target_link_libraries(Editor PRIVATE
		Engine
		${ASSIMP_LIB}
		${SHADERC_LIB}
)


copy_file(Editor ${ASSIMP_DLL} "${CMAKE_CURRENT_BINARY_DIR}/${ASSIMP_DLL_NAME}")
copy_file(Editor ${SHADERC_DLL} "${CMAKE_CURRENT_BINARY_DIR}/${SHADERC_DLL_NAME}")
copy_resources_folder(Editor ${CMAKE_CURRENT_BINARY_DIR})

set(EDITOR_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)