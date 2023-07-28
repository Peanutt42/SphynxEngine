project "Engine"
    kind "SharedLib"
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
		"%{IncludeDirs.yaml}",
		"%{IncludeDirs.glm}",
		"%{IncludeDirs.stbi}",
		"%{IncludeDirs.glfw}",
		"%{IncludeDirs.Vulkan}",
		"%{IncludeDirs.Tracy}",
		"%{IncludeDirs.imgui}"
	}

	libdirs {
		"%{LibaryDirs.Vulkan}"
	}

	links {
		"yaml",
		"stb_image",
		"imgui",
		"glfw",
		"%{Libaries.Vulkan}"
	}

	defines {
		"SE_EXPORT"
	}

	prebuildcommands {
		"cd " .. EngineDir .. "Engine\\Resources\\Shaders",
		"call CompileShaders.bat"
	}

	postbuildcommands {
		"xcopy \"" .. EngineDir .. "bin\\" .. outputdir .. "\\Engine\\Engine.dll\"  \"" .. EngineDir .. "bin\\" .. outputdir ..  "\\Editor\\\" /Y",
		"xcopy \"" .. EngineDir .. "bin\\" .. outputdir .. "\\Engine\\Engine.dll\"  \"" .. EngineDir .. "bin\\" .. outputdir ..  "\\EngineRuntime\\\" /Y"
	}

	
	-- Config dependent links
	filter { "configurations:Debug" }
		links { "%{Libaries.spirv_cross_Debug}", "Tracy" }
	filter { "configurations:Release" }
		links { "%{Libaries.spirv_cross_Release}", "Tracy" }
	filter { "configurations:Dist" }
		links { "%{Libaries.spirv_cross_Dist}" }


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