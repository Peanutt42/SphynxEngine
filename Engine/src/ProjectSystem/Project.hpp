#pragma once

#include "Core/CoreInclude.hpp"
#include "Serialization/YAMLSerializer.hpp"

namespace Sphynx {
	class SE_API Project {
	public:
		std::string Name;
		Version EngineVersion;
		std::filesystem::path Filepath;
		std::filesystem::path Folderpath;
		std::filesystem::path StartSceneFilepath;

		std::filesystem::path EngineConfigFilepath;

		Project(const std::filesystem::path& filepath)
			: Filepath(filepath), Folderpath(Filepath.parent_path()), EngineConfigFilepath(Folderpath / "Config/EngineConfig.ini")
		{
			std::string filepathStr = filepath.string();

			YAML::Node data;
			SE_ASSERT(YAMLSerializer::LoadFile(filepathStr, data), "Failed to open Project file {}", filepathStr);
			Name = data["Name"].as<std::string>();
			EngineVersion = data["EngineVersion"].as<Version>();
			StartSceneFilepath = Folderpath / "Assets" / data["StartScene"].as<std::string>();
		}
	};
}