project "gtest"
    kind "StaticLib"
    language "C++"
    cppdialect "C++14"
	staticruntime "off"

	targetdir("%{wks.location}/bin/" .. outputdir .. "/vendor")
	objdir("%{wks.location}/bin-int/" .. outputdir .. "/vendor")

    files {
        "src/*.h",
        "src/gtest-all.cc"
    }

    includedirs {
        "include",
        "%{prj.location}"
    }
    
	filter { "configurations:Debug" }
        defines { "_DEBUG" }
    filter { "configurations:Release" }
        defines { "NDEBUG" }
    filter { "configurations:Dist" }
        defines { "NDEBUG" }