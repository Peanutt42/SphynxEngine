project "EngineRuntime"
    kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	
	debugdir "$(SolutionDir)"

	targetdir(EngineDir .. "bin/" .. outputdir .. "/%{prj.name}")
	objdir(EngineDir .. "bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "pch.hpp"
    pchsource "src/pch.cpp"

	files {
		"src/**.h",
		"src/**.c",
		"src/**.cpp",
		"src/**.hpp"
	}

	includedirs {
		"src",
		EngineDir .. "Engine/src",
		"%{IncludeDirs.yaml}",
		"%{IncludeDirs.stbi}",
		"%{IncludeDirs.glm}",
		"%{IncludeDirs.glfw}",
		"%{IncludeDirs.Vulkan}",
		"%{IncludeDirs.Tracy}",
		"%{IncludeDirs.imgui}"
	}

	libdirs {
		"%{LibaryDirs.Vulkan}"
	}

	links {
		"Engine",
		"yaml",
		"stb_image",
		"imgui",
		"%{Libaries.Vulkan}"
	}
	
	-- Config dependent links
	filter { "configurations:Debug" }
		links { "%{Libaries.spirv_cross_Debug}", "Tracy" }
		postbuildcommands { "%{CopyBinaryCmds.Tracy}" }
	filter { "configurations:Release" }
		links { "%{Libaries.spirv_cross_Release}", "Tracy" }
		postbuildcommands { "%{CopyBinaryCmds.Tracy}" }
	filter { "configurations:Dist" }
		links { "%{Libaries.spirv_cross_Dist}" }


	filter { "configurations:Debug" }
		runtime "Debug"
		optimize "off"
		symbols "on"
		
	filter { "configurations:Release" }
		runtime "Release"
		optimize "Full"
	
	filter { "configurations:Dist" }
		runtime "Release"
		optimize "Full"

	filter { "system:windows" }
		defines { "WINDOWS" }
	filter { "system:linux" }
		defines { "LINUX" }
	filter { "system:macosx" }
		defines { "MACOS" }

	filter { "configurations:Debug" }
		defines { "DEBUG" }
	filter { "configurations:Release" }
		defines { "RELEASE" }
	filter { "configurations:Dist" }
		defines { "DIST" }