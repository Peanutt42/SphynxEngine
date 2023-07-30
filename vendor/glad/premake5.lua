project "glad"
	kind "StaticLib"
	language "C++"
	cppdialect "C++14"
	staticruntime "off"
	
	debugdir "$(SolutionDir)"

	targetdir("%{wks.location}/bin/" .. outputdir .. "/vendor")
	objdir("%{wks.location}/bin-int/" .. outputdir .. "/vendor")

	files {
		"src/glad.c",
        "include/glad/glad.h"
	}

	includedirs {
        "include"
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