project "Tests"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	debugdir "$(SolutionDir)"

	targetdir(EngineDir .. "bin/" .. outputdir .. "/Programs/%{prj.name}")
	objdir(EngineDir .. "bin-int/" .. outputdir .. "/Programs/%{prj.name}")

	pchheader "pch.hpp"
    pchsource "src/pch.cpp"

	files {
		"src/**.hpp",
		"src/**.cpp"
	}

	includedirs {
		"src",
		"%{IncludeDirs.gtest}",
		EngineDir .. "Engine/src",
		"%{IncludeDirs.yaml}",
		"%{IncludeDirs.glm}",
		"%{IncludeDirs.Tracy}",
		"%{IncludeDirs.imgui}"
	}

	links {
		"gtest",
		"Engine",
		"yaml",
		"imgui"
	}

	
	-- Config dependent links
	filter { "configurations:Debug" }
		links { "Tracy" }
		postbuildcommands { "%{CopyBinaryCmds.Tracy}" }
	filter { "configurations:Release" }
		links { "Tracy" }
		postbuildcommands { "%{CopyBinaryCmds.Tracy}" }

		
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