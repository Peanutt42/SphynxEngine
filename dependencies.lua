IncludeDirs = {}
LibaryDirs = {}
Libaries = {}
CopyBinaryCmds = {}

copydll_cmd_begin = "xcopy \""
copydll_cmd_end = "\"  \"%{cfg.targetdir}\\\" /Y"

VulkanSDKVersion = "1.3.250.1"

-- yaml
IncludeDirs["yaml"] = EngineDir .. "vendor/yaml/include"

-- glfw
IncludeDirs["glfw"] = EngineDir .. "vendor/glfw/include"

-- glm
IncludeDirs["glm"] = EngineDir .. "vendor/glm/include"

-- stbi
IncludeDirs["stbi"] = EngineDir .. "vendor/stb_image/include"

-- Vulkan
IncludeDirs["Vulkan"] = EngineDir .. "vendor/vulkan/%{VulkanSDKVersion}/include"
LibaryDirs["Vulkan"] = EngineDir .. "vendor/vulkan/%{VulkanSDKVersion}/lib"
Libaries["Vulkan"] = "vulkan-1.lib"

-- shaderc
Libaries["shaderc_Debug"] = "shaderc_sharedd.lib"
Libaries["shaderc_Release"] = "shaderc_shared.lib"
Libaries["shaderc_Dist"] = "shaderc_shared.lib"
CopyBinaryCmds["shaderc_Debug"] = copydll_cmd_begin .. EngineDir .. "vendor\\vulkan\\%{VulkanSDKVersion}\\bin\\shaderc_sharedd.dll" .. copydll_cmd_end
CopyBinaryCmds["shaderc_Release"] = copydll_cmd_begin .. EngineDir .. "vendor\\vulkan\\%{VulkanSDKVersion}\\bin\\shaderc_shared.dll" .. copydll_cmd_end
CopyBinaryCmds["shaderc_Dist"] = copydll_cmd_begin .. EngineDir .. "vendor\\vulkan\\%{VulkanSDKVersion}\\bin\\shaderc_shared.dll" .. copydll_cmd_end

-- spirv-cross
Libaries["spirv_cross_Debug"] = "spirv-cross-cored.lib"
Libaries["spirv_cross_Release"] = "spirv-cross-core.lib"
Libaries["spirv_cross_Dist"] = "spirv-cross-core.lib"

-- Tracy
IncludeDirs["Tracy"] = EngineDir .. "vendor/Tracy"
CopyBinaryCmds["Tracy"] = copydll_cmd_begin .. EngineDir .. "bin\\" .. outputdir .. "\\vendor\\Tracy.dll" .. copydll_cmd_end

-- imgui
IncludeDirs["imgui"] = EngineDir .. "vendor/imgui"

-- glad
IncludeDirs["glad"] = EngineDir .. "vendor/glad/include"

-- assimp
IncludeDirs["assimp"] = EngineDir .. "vendor/assimp/include"
LibaryDirs["assimp"] = EngineDir .. "vendor/assimp/lib/%{cfg.buildcfg}"
Libaries["assimp"] = "assimp.lib"
CopyBinaryCmds["assimp"] = copydll_cmd_begin .. EngineDir .. "vendor\\assimp\\bin\\%{cfg.buildcfg}\\assimp.dll" .. copydll_cmd_end

-- gtest
IncludeDirs["gtest"] = EngineDir .. "vendor/gtest/include"

-- bullet
IncludeDirs["bullet"] = EngineDir .. "vendor/bullet3/include"
LibaryDirs["bullet"] = EngineDir .. "vendor/bullet3/lib/%{cfg.buildcfg}"
Libaries["bullet_collision"] = "BulletCollision.lib"
Libaries["bullet_dynamics"] = "BulletDynamics.lib"
Libaries["bullet_math"] = "LinearMath.lib"