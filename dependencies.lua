IncludeDirs = {}
LibaryDirs = {}
Libaries = {}
CopyBinaryCmds = {}


-- yaml
IncludeDirs["yaml"] = "%{wks.location}/vendor/yaml/include"

-- glfw
IncludeDirs["glfw"] = "%{wks.location}/vendor/glfw/include"
LibaryDirs["glfw"] = "%{wks.location}/vendor/glfw/lib"
Libaries["glfw"] = "glfw3.lib"

-- glm
IncludeDirs["glm"] = "%{wks.location}/vendor/glm/include"

-- stbi
IncludeDirs["stbi"] = "%{wks.location}/vendor/stb_image/include"