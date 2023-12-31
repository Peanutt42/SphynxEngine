if(NOT DEFINED VENDOR_DIR)
	set(VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_EXTERNAL OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(LIBRARY_SUFFIX "" CACHE STRING "" FORCE)
set(LIBTYPE "STATIC" CACHE STRING "" FORCE)


if(NOT DEFINED ENGINE_INCLUDE_DIR)
	set(ENGINE_INCLUDE_DIR "${VENDOR_DIR}/../Engine")
endif()

# GLM
set(GLM_INCLUDE_DIR "${VENDOR_DIR}/glm/include")
install(
	DIRECTORY "${GLM_INCLUDE_DIR}/glm"
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# entt
set(ENTT_INCLUDE_DIR "${VENDOR_DIR}/entt/include")
install(
	DIRECTORY "${ENTT_INCLUDE_DIR}/entt"
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# filewatch
set(FILEWATCH_INCLUDE_DIR "${VENDOR_DIR}/filewatch/include")

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_LIBRARY_TYPE "STATIC" CACHE STRING "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# Assimp
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)

# openal-soft
set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ALSOFT_TESTS OFF CACHE BOOL "" FORCE)
set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_CONFIG OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_HRTF_DATA OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_AMBDEC_PRESETS OFF CACHE BOOL "" FORCE)
set(EXTRA_INSTALLS OFF CACHE BOOL "" FORCE)

# Vulkan
find_package(Vulkan REQUIRED)
if (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Could not find Vulkan package!")
else()
	message(STATUS "Using vulkan lib at: ${Vulkan_LIBRARIES}")
	cmake_path(GET Vulkan_LIBRARIES PARENT_PATH  Vulkan_LIB_DIR)
	
	if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
		set(Vulkan_SPIRV_CROSS_LIB "${Vulkan_LIB_DIR}/spirv-cross-core$<$<CONFIG:Debug>:d>.lib")
		set(Vulkan_SHADERC_LIB "${Vulkan_LIB_DIR}/shaderc_combined$<$<CONFIG:Debug>:d>.lib")
	elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
		set(Vulkan_SPIRV_CROSS_LIB "${Vulkan_LIB_DIR}/libspirv-cross-core.a")
		set(Vulkan_SHADERC_LIB "shaderc_combined")
	endif()
endif()


# bullet3
set(BULLET3_INCLUDE_DIR "${VENDOR_DIR}/bullet3/src")
set(BULLET3_LIBS BulletDynamics BulletCollision LinearMath)
set(USE_GRAPHICAL_BENCHMARK OFF CACHE BOOL "" FORCE)
set(BUILD_CPU_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(INSTALL_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(INSTALL_CMAKE_FILES OFF CACHE BOOL "" FORCE)
set(BUILD_ENET OFF CACHE BOOL "" FORCE)
set(BUILD_CLSOCKET OFF CACHE BOOL "" FORCE)
set(OpenGL_GL_PREFERENCE "GLVND")


if (NOT DEFINED VENDOR_NO_PARENT)
	set(ENGINE_INCLUDE_DIR ${ENGINE_INCLUDE_DIR} PARENT_SCOPE)
	set(GLM_INCLUDE_DIR ${GLM_INCLUDE_DIR} PARENT_SCOPE)
	set(FILEWATCH_INCLUDE_DIR ${FILEWATCH_INCLUDE_DIR} PARENT_SCOPE)
	set(ENTT_INCLUDE_DIR ${ENTT_INCLUDE_DIR} PARENT_SCOPE)
	set(OPENGL_LIB ${OPENGL_LIB} PARENT_SCOPE)
	set(ASSIMP_LIB ${ASSIMP_LIB} PARENT_SCOPE)
	set(Vulkan_INCLUDE_DIRS ${Vulkan_INCLUDE_DIRS} PARENT_SCOPE)
	set(Vulkan_LIBRARIES ${Vulkan_LIBRARIES} PARENT_SCOPE)
	set(Vulkan_LIB_DIR ${Vulkan_LIB_DIR} PARENT_SCOPE)
	set(Vulkan_SPIRV_CROSS_LIB ${Vulkan_SPIRV_CROSS_LIB} PARENT_SCOPE)
	set(Vulkan_SHADERC_LIB ${Vulkan_SHADERC_LIB} PARENT_SCOPE)
	set(BULLET3_INCLUDE_DIR ${BULLET3_INCLUDE_DIR} PARENT_SCOPE)
	set(BULLET3_LIBS ${BULLET3_LIBS} PARENT_SCOPE)
	set(OpenGL_GL_PREFERENCE ${OpenGL_GL_PREFERENCE} PARENT_SCOPE)
endif()