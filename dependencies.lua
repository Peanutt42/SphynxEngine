IncludeDirs = {}
LibaryDirs = {}
Libaries = {}
CopyBinaryCmds = {}

VulkanSDK = os.getenv("VULKAN_SDK")

-- yaml
IncludeDirs["yaml"] = "%{wks.location}/vendor/yaml/include"

-- glfw
IncludeDirs["glfw"] = "%{wks.location}/vendor/glfw/include"
LibaryDirs["glfw"] = "%{wks.location}/vendor/glfw/lib"
Libaries["glfw"] = "glfw3.lib"

-- glm
IncludeDirs["glm"] = "%{wks.location}/vendor/glm/include"

-- stbi
IncludeDirs["stbi"] = "%{wks.location}/vendor/stb_image/include"

-- Optick
IncludeDirs["optick"] = "%{wks.location}/vendor/Optick/include"
LibaryDirs["optick"] = "%{wks.location}/vendor/Optick/lib/%{cfg.buildcfg}"
Libaries["optick"] = "OptickCore.lib"

-- Vulkan
IncludeDirs["Vulkan"] = "%{VulkanSDK}/Include"
LibaryDirs["Vulkan"] = "%{VulkanSDK}/Lib"
Libaries["Vulkan"] = "vulkan-1.lib"

-- shaderc
Libaries["shaderc_Debug"] = "%{LibaryDirs.Vulkan}/shaderc_sharedd.lib"
Libaries["shaderc_Release"] = "%{LibaryDirs.Vulkan}/shaderc_shared.lib"
Libaries["shaderc_Dist"] = "%{LibaryDirs.Vulkan}/shaderc_shared.lib"

-- spirv-cross
Libaries["spirv_cross_Debug"] = "%{LibaryDirs.Vulkan}/spirv-cross-cored.lib"
Libaries["spirv_cross_Release"] = "%{LibaryDirs.Vulkan}/spirv-cross-core.lib"
Libaries["spirv_cross_Dist"] = "%{LibaryDirs.Vulkan}/spirv-cross-core.lib"