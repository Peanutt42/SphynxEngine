set(TRACY_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)

add_library(tracy STATIC TracyClient.cpp tracy/Tracy.hpp)

target_compile_definitions(tracy PRIVATE
		TRACY_ENABLE
		TRACY_EXPORTS
		TRACY_ON_DEMAND
)

target_compile_features(tracy PRIVATE cxx_std_17)