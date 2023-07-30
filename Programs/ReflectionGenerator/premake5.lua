project "ReflectionGenerator"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	debugdir "$(SolutionDir)"

	targetdir("bin")
	objdir("bin/int")

	files {
		"src/**.hpp",
		"src/**.cpp"
	}

	includedirs {
		"src"
	}

	defines {

	}

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