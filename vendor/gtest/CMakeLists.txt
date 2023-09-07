file(GLOB_RECURSE GTEST_HEADERS "src/*.h")

add_library(gtest STATIC "src/gtest-all.cc" ${GTEST_HEADERS})

target_include_directories(gtest PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		"include"
)

target_compile_features(gtest PRIVATE cxx_std_14)