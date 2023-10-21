## assimp
- Link: https://github.com/assimp/assimp
- Commit: 3cf7d28bc

## bullet3
- Link: https://github.com/bulletphysics/bullet3
- Release: 3.25
- Build CMAKE_BUILD_TYPE as Debug and Release and set CMAKE_POSITION_INDEPENDENT_CODE to TRUE
- Copy /src/ into vendor/bullet3/include
- Copy library files from BulletCollision, BulletDynamics and LinearMath inside the build/src/ folder into vendor/bullet3/lib/$Platform/$Config

## backward-cpp
- Link: https://github.com/bombela/backward-cpp/tree/master
- Commit: 65a769f
- Just copy the backward.hpp, the CMakeLists.txt is our own

## entt
- Link 
- Release: 3.12.2
- Just copy /single_include/entt/entt.hpp into /vendor/entt/include/entt/entt.hpp

## filewatch
- Link: https://github.com/ThomasMonkman/filewatch
- Commit: a59891b

## fmt
- Link: https://github.com/fmtlib/fmt/tree/master
- Commit: d9063ba

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
