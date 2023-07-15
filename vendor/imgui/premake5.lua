project "imgui"
	kind "StaticLib"
	language "C++"
	cppdialect "C++14"
	staticruntime "off"
	
	targetdir(EngineDir .. "bin/" .. outputdir .. "/vendor")
	objdir(EngineDir ..  "bin-int/" .. outputdir .. "/vendor")

	files {
		"include/**.h",
		"include/**.c",
		"include/**.cpp",
		"include/**.hpp"
	}

	includedirs {
		"include",
		EngineDir .. "vendor/glfw/include",
		VulkanSDK .. "/Include"
	}

	defines {

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