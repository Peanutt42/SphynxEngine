project "Tracy"
	kind "SharedLib"
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
        "TRACY_ENABLE",
		"TRACY_EXPORTS",
		"TRACY_ON_DEMAND"
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