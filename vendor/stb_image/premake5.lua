project "stb_image"
	kind "StaticLib"
	language "C++"
	cppdialect "C++14"
	staticruntime "off"

	targetdir("%{wks.location}/bin/" .. outputdir .. "/vendor")
	objdir("%{wks.location}/bin-int/" .. outputdir .. "/vendor")

	files {
		"include/**.h",
		"include/**.cpp"
	}

	includedirs {
		"include"
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