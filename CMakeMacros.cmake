macro(copy_resources_folder _TARGET BIN_DIR)
	add_custom_command(
			TARGET ${_TARGET}
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${SE_RESOURCE_DIR} "${BIN_DIR}/Resources"
			COMMENT "Copying ${SE_RESOURCE_DIR} to ${BIN_DIR}/Resources"
	)
endmacro()

macro(copy_file _TARGET SOURCE DEST)
	add_custom_command(
			TARGET ${_TARGET}
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SOURCE}" "${DEST}"
			COMMENT "Copying ${SOURCE} to ${DEST}"
	)
endmacro()

macro(define_configuration _TARGET)
	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_compile_definitions(${_TARGET} PUBLIC DEBUG _DEBUG)
	elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
		target_compile_definitions(${_TARGET} PUBLIC RELEASE NDEBUG)
	elseif (CMAKE_BUILD_TYPE STREQUAL "Dist")
		target_compile_definitions(${_TARGET} PUBLIC DIST NDEBUG)
	else()
		target_compile_definitions(${_TARGET} PUBLIC NDEBUG)
	endif()
endmacro()
macro(define_platform _TARGET)
	if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
		target_compile_definitions(${_TARGET} PUBLIC WINDOWS)
	elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
		target_compile_definitions(${_TARGET} PUBLIC LINUX)
	elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
		target_compile_definitions(${_TARGET} PUBLIC MACOS)
	else()
		message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
	endif()
endmacro()