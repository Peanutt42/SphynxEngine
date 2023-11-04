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

		std::filesystem::path EngineConfigFilepath;
		std::filesystem::path BinariesDirectory;
		std::filesystem::path CacheFolder;

		Project(const std::filesystem::path& filepath)
			: Filepath(filepath), Folderpath(Filepath.parent_path()), EngineConfigFilepath(Folderpath / "Config/EngineConfig.ini"),
			BinariesDirectory(Folderpath / "Binaries"), CacheFolder(Folderpath / "Cache")
		{
			if (!std::filesystem::exists(CacheFolder)) std::filesystem::create_directory(CacheFolder);

			std::string filepathStr = filepath.string();

			YAML::Node data = YAMLSerializer::LoadFile(filepathStr).expect("Failed to open Project file");
			Name = data["Name"].as<std::string>();
			EngineVersion = data["EngineVersion"].as<Version>();
			StartSceneFilepath = Folderpath / "Assets" / data["StartScene"].as<std::string>();
			GameModuleName = data["GameModule"].as<std::string>();
		}
	};
}