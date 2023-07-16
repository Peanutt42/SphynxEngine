project "CrashReporter"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	
	debugdir "bin"

	targetdir("bin")
	objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files {
		"src/**.h",
		"src/**.c",
		"src/**.cpp",
		"src/**.hpp"
	}

	includedirs {
		"src",
		"%{IncludeDirs.imgui}",
		"%{IncludeDirs.glfw}",
		"%{IncludeDirs.yaml}",
		"%{IncludeDirs.glad}",
	}

	libdirs {
		"%{LibaryDirs.glfw}",
		"%{wks.location}/bin/" .. outputdir .. "/vendor"
	}

	links {
		"%{Libaries.glfw}",
		"imgui",
		"yaml",
		"glad",
		"opengl32.lib"
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