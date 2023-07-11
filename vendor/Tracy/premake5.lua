project "Tracy"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"
	editandcontinue "off"

	targetdir("%{wks.location}/bin/" .. outputdir .. "/vendor")
	objdir("%{wks.location}/bin-int/" .. outputdir .. "/vendor")

	files {
		"tracy/Tracy.hpp",
		"TracyClient.cpp",
	}

	includedirs {
		--"include"
	}

	defines {
        "TRACY_ENABLE"
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