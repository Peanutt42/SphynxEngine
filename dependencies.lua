IncludeDirs = {}
LibaryDirs = {}
Libaries = {}
CopyBinaryCmds = {}

VulkanSDK = os.getenv("VULKAN_SDK")

-- yaml
IncludeDirs["yaml"] = EngineDir .. "vendor/yaml/include"

-- glfw
IncludeDirs["glfw"] = EngineDir .. "vendor/glfw/include"
LibaryDirs["glfw"] = EngineDir .. "vendor/glfw/lib"
Libaries["glfw"] = "glfw3.lib"

-- glm
IncludeDirs["glm"] = EngineDir .. "vendor/glm/include"

-- stbi
IncludeDirs["stbi"] = EngineDir .. "vendor/stb_image/include"

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

-- Tracy
IncludeDirs["Tracy"] = EngineDir .. "vendor/Tracy"