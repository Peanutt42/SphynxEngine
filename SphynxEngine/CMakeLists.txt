file(GLOB_RECURSE SPHYNX_ENGINE_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)
file(GLOB_RECURSE SPHYNX_ENGINE_HEADERS
		${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
)

add_library(SphynxEngine SHARED ${SPHYNX_ENGINE_SOURCES} ${SPHYNX_ENGINE_HEADERS})

if(NOT DEFINED SPHYNX_ENGINE_INLCUDE_DIR)
	set(SPHYNX_ENGINE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
	set(SPHYNX_ENGINE_INCLUDE_DIR ${SPHYNX_ENGINE_INCLUDE_DIR} PARENT_SCOPE)
elseif(${SPHYNX_ENGINE_INCLUDE_DIR} NOT STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
	message(FATAL_ERROR "Previous SPHYNX_ENGINE_INCLUDE_DIR was invalid!")
endif()

target_include_directories(SphynxEngine
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		$<BUILD_INTERFACE:${GLM_INCLUDE_DIR}>
	PRIVATE
		${FILEWATCH_INCLUDE_DIR}
		${IMGUI_INCLUDE_DIR}
		${BULLET_INCLUDE_DIR}
		${Vulkan_INCLUDE_DIRS}
)

add_dependencies(SphynxEngine
		yaml
		stb_image
		imgui
		glfw
		tracy
)

target_link_libraries(SphynxEngine
	PUBLIC
		yaml
		stb_image
		imgui
		tracy
	PRIVATE
		${Vulkan_LIBRARIES}
		${Vulkan_SPIRV_CROSS_LIB}
		${BULLET_COLLISION_LIB}
		${BULLET_DYNAMICS_LIB}
		${BULLET_MATH_LIB}
		sndfile
		OpenAL
		glfw
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	target_link_libraries(SphynxEngine PRIVATE Dbghelp.lib)
endif()

target_compile_definitions(SphynxEngine PRIVATE "SE_EXPORT")

target_compile_features(SphynxEngine PRIVATE cxx_std_20)

target_precompile_headers(SphynxEngine PUBLIC $<BUILD_INTERFACE:pch.hpp>)

define_configuration(SphynxEngine)
define_platform(SphynxEngine)


add_custom_command(TARGET SphynxEngine
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:SphynxEngine> $<TARGET_FILE_DIR:SphynxEditor> &&
			${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:SphynxEngine> $<TARGET_FILE_DIR:SphynxRuntime> &&
			${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:SphynxEngine> $<TARGET_FILE_DIR:Tests>
)

# Install
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_SOURCE_DIR}/cmake/SphynxEngineConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/SphynxEngineConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/SphynxEngine"
)

install(
	TARGETS SphynxEngine yaml stb_image imgui tracy
	EXPORT SphynxEngineTargets
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
	DIRECTORY ${SPHYNX_ENGINE_INCLUDE_DIR}
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	FILES_MATCHING PATTERN "*.hpp"
)

install(
	FILES
		"${CMAKE_CURRENT_BINARY_DIR}/SphynxEngineConfig.cmake"
		"${CMAKE_SOURCE_DIR}/cmake/SphynxMacros.cmake"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/SphynxEngine"
)

install(
	EXPORT SphynxEngineTargets
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/SphynxEngine"
)