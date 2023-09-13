# SphynxEngine
Why 'Sphynx'? Because I love cats and this game engine isn't here to replace UE5

## Prerequisites (Optional)
- Vulkan SDK: only needed for Vulkan Debugging (validation layers) -> https://vulkan.lunarg.com/sdk/home

## Building
Run premake5 for your operating system
- Windows:
  - Run 'Scripts/BuildProjectWin.bat' to generate the Visual Studio 2022 project files
  - Open 'SphynxEngine.sln'
- Other:
  - While not currently supported, adding new platforms shouldn't be too hard since i made (mostly) all platform specific stuff hidden in PlatformImplementations (see Engine/src/Platform/WindowsPlatform.cpp)
