## assimp
- Link: https://github.com/assimp/assimp
- Commit: 3cf7d28bc

## backward-cpp
- Link: https://github.com/bombela/backward-cpp/tree/master
- Commit: 65a769f
- Just copy the backward.hpp, the CMakeLists.txt is our own

## filewatch
- Link: https://github.com/ThomasMonkman/filewatch
- Commit: a59891b

## glfw
- Link: https://github.com/Peanutt42/glfw
- Commit: c186bef66

## gtest
- Link: https://github.com/google/googletest
- Commit: beb552f
- just copy the source code from googletest/src and googletest/include into the vendor/gtest and leave the previous CMakeLists.txt (custom)

## imgui
- Link: https://github.com/ocornut/imgui/tree/docking
- Commit: 83c4336
- leave the CMakeLists.txt (or change it if needed)

## libsndfile
- Link: https://github.com/libsndfile/libsndfile
- Commit: b2e547f70
- Note: remove "include/sndfile.h" from the .gitignore

## openal-soft
- Link: https://github.com/kcat/openal-soft
- Commit: 2ea092d6a

## shaderc
- Link: https://github.com/Peanutt42/shaderc
- Commit: 16f3864ec
- Make sure you clone the submodule as well:
    - shaderc/third_party/glslang
    - shaderc/third_party/spirv-headers
    - shaderc/third_party/spirv-tools