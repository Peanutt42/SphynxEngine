![example workflow](https://github.com/Peanutt42/SphynxEngine/actions/workflows/windows%20build.yml/badge.svg)

# SphynxEngine
Why 'Sphynx'? Because I love cats and this game engine isn't here to replace UE5

## Prerequisites (Optional)
- Vulkan SDK: only needed for Vulkan Debugging (validation layers) -> https://vulkan.lunarg.com/sdk/home

## Building
We use cmake for our build system
- Windows:
  - you can use CLion or use cmake to build it
- Other:
  - While not currently supported, adding new platforms shouldn't be too hard since i made (mostly) all platform specific stuff hidden in PlatformImplementations (see Engine/src/Platform/WindowsPlatform.cpp)
