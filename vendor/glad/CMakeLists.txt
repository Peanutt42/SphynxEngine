file(GLOB_RECURSE GLAD_HEADERS
		include/*.h
)

add_library(glad STATIC "src/glad.c" ${GLAD_HEADERS})

target_include_directories(glad PUBLIC "include")

target_compile_features(glad PRIVATE cxx_std_14)