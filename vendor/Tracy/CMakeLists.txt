add_library(tracy STATIC TracyClient.cpp tracy/Tracy.hpp)

target_include_directories(tracy PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_compile_definitions(tracy PRIVATE
		TRACY_ENABLE
		TRACY_ON_DEMAND
)

target_compile_features(tracy PRIVATE cxx_std_17)