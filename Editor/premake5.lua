project "Editor"
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
		"%{IncludeDirs.glm}",
		"%{IncludeDirs.Tracy}",
		"%{IncludeDirs.imgui}",
		"%{IncludeDirs.assimp}",
		"%{IncludeDirs.Vulkan}"
	}

	libdirs {
		"%{LibaryDirs.assimp}",
		"%{LibaryDirs.Vulkan}"
	}

	links {
		"Engine",
		"yaml",
		"imgui",
		"%{Libaries.assimp}"
	}

	postbuildcommands {
		"%{CopyBinaryCmds.assimp}"
	}
	
	-- Config dependent links
	filter { "configurations:Debug" }
		links { "%{Libaries.shaderc_Debug}", "Tracy" }
		postbuildcommands { "%{CopyBinaryCmds.shaderc_Debug}", "%{CopyBinaryCmds.Tracy}" }
	filter { "configurations:Release" }
		links { "%{Libaries.shaderc_Release}", "Tracy" }
		postbuildcommands { "%{CopyBinaryCmds.shaderc_Release}", "%{CopyBinaryCmds.Tracy}" }
	filter { "configurations:Dist" }
		links { "%{Libaries.shaderc_Dist}" }
		postbuildcommands { "%{CopyBinaryCmds.shaderc_Dist}" }


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