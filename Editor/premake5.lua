project "Editor"
    kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	
	debugdir "$(SolutionDir)"

	targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

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
		"%{wks.location}/Engine/src",
		"%{IncludeDirs.yaml}",
		"%{IncludeDirs.stbi}",
		"%{IncludeDirs.glm}",
		"%{IncludeDirs.glfw}",
		"%{IncludeDirs.Vulkan}",
		"%{IncludeDirs.Tracy}"
	}

	links {
		"Engine",
		"yaml",
		"stb_image",
		"tracy"
	}
	
	-- Config dependent links
	filter { "configurations:Debug" }
		links { "%{Libaries.shaderc_Debug}", "%{Libaries.spirv_cross_Debug}" }
	filter { "configurations:Release" }
		links { "%{Libaries.shaderc_Release}", "%{Libaries.spirv_cross_Release}" }
	filter { "configurations:Dist" }
		links { "%{Libaries.shaderc_Dist}", "%{Libaries.spirv_cross_Dist}" }


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