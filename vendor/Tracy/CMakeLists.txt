add_library(tracy STATIC TracyClient.cpp tracy/Tracy.hpp)

target_include_directories(tracy PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_compile_definitions(tracy PRIVATE
		TRACY_ENABLE
		TRACY_ON_DEMAND
)

target_compile_features(tracy PRIVATE cxx_std_17)

set_target_properties(tracy PROPERTIES POSITION_INDEPENDENT_CODE ON)


install(
	DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)