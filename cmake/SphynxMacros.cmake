macro(copy_content_folder _TARGET BIN_DIR)
	add_custom_command(
			TARGET ${_TARGET}
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E echo "Copying Content folder to ${BIN_DIR}/Content"
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${SE_CONTENT_DIR} "${BIN_DIR}/Content"
	)
endmacro()

macro(copy_file _TARGET SOURCE DEST)
	add_custom_command(
			TARGET ${_TARGET}
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E echo "Copying ${SOURCE} to ${DEST}"
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SOURCE}" "${DEST}"
	)
endmacro()


macro(define_configuration _TARGET)
	target_compile_definitions(${_TARGET} PUBLIC $<IF:$<CONFIG:Debug>,DEBUG _DEBUG,NDEBUG>)
	target_compile_definitions(${_TARGET} PUBLIC $<$<CONFIG:Release>:RELEASE>)
	target_compile_definitions(${_TARGET} PUBLIC $<$<CONFIG:RelWithDebInfo>:DEVELOPMENT>)
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