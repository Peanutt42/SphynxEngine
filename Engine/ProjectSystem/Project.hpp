#pragma once

#include "pch.hpp"
#include "Serialization/YAMLSerializer.hpp"

namespace Sphynx {
	class SE_API Project {
	public:
		std::string Name;
		Version EngineVersion;
		std::filesystem::path Filepath;
		std::filesystem::path Folderpath;
		std::filesystem::path StartSceneFilepath;
		std::filesystem::path BinaryFilepath;

		std::filesystem::path EngineConfigFilepath;

		Project(const std::filesystem::path& filepath)
			: Filepath(filepath), Folderpath(Filepath.parent_path()), EngineConfigFilepath(Folderpath / "Config/EngineConfig.ini")
		{
			std::string filepathStr = filepath.string();

			auto result = YAMLSerializer::LoadFile(filepathStr);
			result.expect("Failed to open Project file");
			YAML::Node& data = *result;
			Name = data["Name"].as<std::string>();
			EngineVersion = data["EngineVersion"].as<Version>();
			StartSceneFilepath = Folderpath / "Assets" / data["StartScene"].as<std::string>();
			BinaryFilepath = Folderpath / data["Binary"].as<std::string>();
		}
	};
}