#pragma once

#include "pch.hpp"
#include "Serialization/YAMLSerializer.hpp"

namespace Sphynx {
	class Project {
	public:
		std::string Name;
		Version EngineVersion;
		std::filesystem::path Filepath;
		std::filesystem::path Folderpath;
		std::filesystem::path StartSceneFilepath;
		std::string GameModuleName;

		std::filesystem::path BinariesDirectory;

		Project(const std::filesystem::path& filepath)
			: Filepath(filepath), Folderpath(Filepath.parent_path()),
			BinariesDirectory(Folderpath / "Binaries")
		{
			std::string filepathStr = filepath.string();

			YAML::Node data = YAMLSerializer::LoadFile(filepathStr).expect("Failed to open Project file");
			Name = data["Name"].as<std::string>();
			EngineVersion = data["EngineVersion"].as<Version>();
			StartSceneFilepath = Folderpath / "Assets" / data["StartScene"].as<std::string>();
			GameModuleName = data["GameModule"].as<std::string>();
		}
	};
}