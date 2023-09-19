# SphynxEngine
Why 'Sphynx'? Because I love cats and this game engine is bare bones

## Building
- Download Vulkan SDK: https://vulkan.lunarg.com/sdk/home
- Use cmake (better in a IDE)
- We use cmake for our build system but currently only support Windows.
- To support new platforms:
    - install vulkan sdk
    - (for now) build bullet3
    - implement the platform specific implementation (see Engine/src/Platform/Platform.hpp)