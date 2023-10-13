#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <fstream>

namespace Sphynx::ReflectionGenerator {
    struct VariableReflectionInfo {
        std::string Type;
        std::string Name;

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
		static bool Serialize(const std::filesystem::path& filepath, const std::vector<ComponentReflectionInfo>& components, const std::vector<std::filesystem::path>& filepaths, const std::filesystem::path& srcDir);

        static bool LoadUnchanged(const std::filesystem::path& filepath, const std::filesystem::path& srcDir, std::vector<std::filesystem::path>& outSrcFilepaths, std::vector<ComponentReflectionInfo>& outComponents, std::vector<std::filesystem::path>& outChangedFilepaths);
	};
}