project "gtest"
    language "C++"
    cppdialect "C++14"
    kind "StaticLib"

	targetdir("%{wks.location}/bin/" .. outputdir .. "/vendor")
	objdir("%{wks.location}/bin-int/" .. outputdir .. "/vendor")

    includedirs {
        "include",
        "%{prj.location}"
    }

    files {
        "src/*.h",
        "src/*.cc"
    }