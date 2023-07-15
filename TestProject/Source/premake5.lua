EngineDir = "../../"

include "../../dependencies.lua"

workspace "TestProject"
    architecture "x64"

    configurations {
        "Debug",
        "Release",
        "Dist"
    }

    flags {
        "MultiProcessorCompile",
        "LinkTimeOptimization"
	}

	editandcontinue "Off"

    warnings "Extra" -- /W4

    startproject "TestProject"



project "TestProject"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir("../Binaries")
	objdir("../Binaries/Intermediates")

	files {
		"**.h",
		"**.hpp",
		"**.cpp"
	}

	includedirs {
		"TestProject",
		EngineDir .. "Engine/src",
		"%{IncludeDirs.yaml}",
		"%{IncludeDirs.glm}",
		"%{IncludeDirs.stbi}",
		"%{IncludeDirs.glfw}",
		"%{IncludeDirs.Vulkan}",
		"%{IncludeDirs.Tracy}"
	}

	defines {
		"SE_GAME_MODULE"
	}

	prebuildcommands {
		"ReflectionGenerator.bat"
	}

	filter { "configurations:Debug" }
		buildoptions "/MDd"
		runtime "Debug"
		optimize "off"
		symbols "on"
		
	filter { "configurations:Release" }
		buildoptions "/MD"
		runtime "Release"
		optimize "Full"
	
	filter { "configurations:Dist" }
		buildoptions "/MD"
		runtime "Release"
		optimize "Full"