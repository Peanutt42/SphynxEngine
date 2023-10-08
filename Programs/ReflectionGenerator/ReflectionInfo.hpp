#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <filesystem>
#include <fstream>
#include "yaml-cpp/yaml.h"

namespace Sphynx::ReflectionGenerator {
    struct VariableReflectionInfo {
        std::string Name;
        std::string Type;

        // set in game module, not by generator
        size_t Offset = 0;
    };

    struct ComponentReflectionInfo {
        std::string Fullname;
		std::filesystem::path Filepath;
        std::vector<VariableReflectionInfo> Variables;

        // set in game module, not by generator
        size_t Size = 0;
        void(*Constructor)(void* _this) = nullptr;
        void(*Destructor)(void* _this) = nullptr;
        void(*Copy)(void* _this, void* other) = nullptr;
    };

	struct CachedReflectionInfo {
		static bool Serialize(const std::filesystem::path& filepath, const std::vector<ComponentReflectionInfo>& components, const std::vector<std::filesystem::path>& filepaths, const std::filesystem::path& srcDir) {
			YAML::Emitter out;
			out << YAML::BeginMap;

			out << YAML::Key << "Files" << YAML::Value << YAML::BeginSeq;
			for (const std::filesystem::path& filepath : filepaths) {
				out << YAML::BeginMap;
				out << YAML::Key << std::filesystem::relative(filepath, srcDir).string() << YAML::Value << std::filesystem::last_write_time(filepath).time_since_epoch().count();
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;
			
			out << YAML::Key << "Components" << YAML::Value << YAML::BeginSeq;
			for (const auto& component : components) {
				out << YAML::BeginMap;
				out << YAML::Key << "Name" << YAML::Value << component.Fullname;
				out << YAML::Key << "Filepath" << YAML::Value << std::filesystem::relative(component.Filepath, srcDir).string();
				out << YAML::Key << "Variables" << YAML::Value << YAML::BeginSeq;
				for (const auto& var : component.Variables) {
					out << YAML::BeginMap;
					out << YAML::Key << var.Name << YAML::Value << var.Type;
					out << YAML::EndMap;
				}
				out << YAML::EndSeq;
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;

			out << YAML::EndMap;

			std::ofstream cacheFile(filepath);
			if (cacheFile.is_open()) {
				cacheFile << out.c_str();
				return true;
			}
			return false;
		}

		static bool LoadUnchanged(const std::filesystem::path& filepath, const std::filesystem::path& srcDir, std::vector<std::filesystem::path>& outSrcFilepaths, std::vector<ComponentReflectionInfo>& outComponents, std::vector<std::filesystem::path>& outChangedFilepaths) {
			if (!std::filesystem::exists(filepath))
				return false;

			std::unordered_map<std::filesystem::path, long long> filepathWriteTimeMap;
			std::vector<ComponentReflectionInfo> cachedComponents;
			for (const auto& directory_iter : std::filesystem::recursive_directory_iterator(srcDir)) {
				if (directory_iter.is_directory())
					continue;
				auto path = directory_iter.path();
				auto extension = path.extension();
				if (extension == ".h" || extension == ".c" || extension == ".cc" || extension == ".hpp" || extension == ".cpp")
					outSrcFilepaths.push_back(path);
			}

			try {
				YAML::Node cache = YAML::LoadFile(filepath.string());

				for (YAML::Node filepathNode : cache["Files"]) {
					if (filepathNode.IsMap() && filepathNode.size() == 1)
						filepathWriteTimeMap[filepathNode.begin()->first.as<std::string>()] = filepathNode.begin()->second.as<long long>();
				}
				for (YAML::Node componentNode : cache["Components"]) {
					ComponentReflectionInfo& cachedComponent = cachedComponents.emplace_back();
					cachedComponent.Fullname = componentNode["Name"].as<std::string>();
					cachedComponent.Filepath = componentNode["Filepath"].as<std::string>();
					for (YAML::Node variableNode : componentNode["Variables"]) {
						if (variableNode.IsMap() && variableNode.size() == 1) {
							auto& cachedVar = cachedComponent.Variables.emplace_back();
							cachedVar.Name = variableNode.begin()->first.as<std::string>();
							cachedVar.Type = variableNode.begin()->second.as<std::string>();
						}
					}
				}
			}
			catch (const std::exception& e) {
				std::cout << "Failed to load last cache: " << e.what() << ", generating from scratch\n";
				return false;
			}

			for (const auto& srcFilepath : outSrcFilepaths) {
				const auto relativeSrcFilepath = std::filesystem::relative(srcFilepath, srcDir);
				auto findCachedLastWriteTime = filepathWriteTimeMap.find(relativeSrcFilepath);
				auto srcLastWriteTime = std::filesystem::last_write_time(srcFilepath).time_since_epoch().count();
				if (findCachedLastWriteTime == filepathWriteTimeMap.end() || srcLastWriteTime != findCachedLastWriteTime->second)
					outChangedFilepaths.push_back(srcFilepath);
				else {
					for (const auto& cachedComponent : cachedComponents) {
						if (cachedComponent.Filepath != relativeSrcFilepath)
							continue;

						outComponents.push_back(cachedComponent);
						outComponents.back().Filepath = srcDir / cachedComponent.Filepath;
					}
				}
			}

			return true;
		}
	};
}