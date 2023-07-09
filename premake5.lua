include "dependencies.lua"

workspace "SphynxEngine"
    architecture "x64"

    configurations {
        "Debug",
        "Release",
        "Dist"
    }

    flags {
        "MultiProcessorCompile",
        "LinkTimeOptimization"
	}

	editandcontinue "Off"

    warnings "Extra" -- /W4

    startproject "Editor"

	outputdir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

group "Core"
	include "Editor"
	include "Engine"

group "Dependencies"
	include "vendor/yaml"

newaction {
	trigger = "clean",
	description = "Remove all binaries and intermediate binaries, and vs files.",
	execute = function()
		print("Removing binaries")
		os.rmdir("./bin")
		print("Removing intermediate binaries")
		os.rmdir("./bin-int")
		print("Removing project files")
		os.remove("**.sln")
		os.remove("**.vcxproj")
		os.remove("**.vcxproj.filters")
		os.remove("**.vcxproj.user")
		print("Done")
	end
}