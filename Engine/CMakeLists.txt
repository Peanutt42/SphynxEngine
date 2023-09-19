file(GLOB_RECURSE ENGINE_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)
file(GLOB_RECURSE ENGINE_HEADERS
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
)

add_library(Engine SHARED ${ENGINE_SOURCES} ${ENGINE_HEADERS})

if(NOT DEFINED ENGINE_INLCUDE_DIR)
	set(ENGINE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
	set(ENGINE_INCLUDE_DIR ${ENGINE_INCLUDE_DIR} PARENT_SCOPE)
elseif(${ENGINE_INCLUDE_DIR} NOT STREQUAL ${CMAKE_CURRENT_SOURCE_DIR}/src)
	message(FATAL_ERROR "Previous ENGINE_INCLUDE_DIR was invalid!")
endif()

target_include_directories(Engine PUBLIC
		${ENGINE_INCLUDE_DIR}
		${GLM_INCLUDE_DIR}
		${TRACY_INCLUDE_DIR}
		${IMGUI_INCLUDE_DIR}
		${Vulkan_INCLUDE_DIRS}
		${BULLET_INCLUDE_DIR}
)

add_dependencies(Engine
		yaml
		stb_image
		imgui
		glfw
		tracy
)

target_link_libraries(Engine PUBLIC
		yaml
		stb_image
		imgui
		glfw
		tracy
)
target_link_libraries(Engine PRIVATE
		${Vulkan_LIBRARIES}
		${Vulkan_SPIRV_CROSS_LIB}
		${BULLET_COLLISION_LIB}
		${BULLET_DYNAMICS_LIB}
		${BULLET_MATH_LIB}
		sndfile
		OpenAL
)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	target_link_libraries(Engine PRIVATE Dbghelp.lib)
endif()

target_compile_features(Engine PRIVATE cxx_std_20)

target_compile_definitions(Engine PRIVATE SE_EXPORT)

target_precompile_headers(Engine PUBLIC src/pch.hpp)

define_configuration(Engine)
define_platform(Engine)

set(ENGINE_DLL "${CMAKE_CURRENT_BINARY_DIR}/Engine.dll")
copy_file(Engine ${ENGINE_DLL} "${CMAKE_BINARY_DIR}/Editor/Engine.dll")
copy_file(Engine ${ENGINE_DLL} "${CMAKE_BINARY_DIR}/EngineRuntime/Engine.dll")
copy_file(Engine ${ENGINE_DLL} "${CMAKE_BINARY_DIR}/Programs/Tests/Engine.dll")