set(STBI_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)

add_library(stb_image STATIC include/stb_image.cpp include/stb_image.h)

target_include_directories(stb_image PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(stb_image PRIVATE cxx_std_14)