file(GLOB_RECURSE YAML_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)
file(GLOB_RECURSE YAML_HEADERS
		${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
)

set(YAML_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)

add_library(yaml STATIC ${YAML_SOURCES} ${YAML_HEADERS})

target_include_directories(yaml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(yaml PRIVATE cxx_std_14)