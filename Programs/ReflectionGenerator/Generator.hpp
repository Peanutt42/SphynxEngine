#pragma once

#include "Parser.hpp"
#include <filesystem>
#include <fstream>

namespace Sphynx::ReflectionGenerator {
	class Generator {
	public:
		static void Generate(const std::vector<ComponentReflectionInfo>& components, const std::vector<std::filesystem::path>& filepaths, const std::filesystem::path& srcDir, const std::filesystem::path& outputDir, const std::string& engineDir) {
			std::ofstream outCppFile(outputDir / "generated.cpp");
			if (!outCppFile.is_open())
				return;

			outCppFile << "#include \"Core/CoreInclude.hpp\"\n";
			outCppFile << "#include \"" << engineDir << "Programs/ReflectionGenerator/ReflectionInfo.hpp\"\n";
			outCppFile << "#include \"" << engineDir << "Engine/Scene/Scene.hpp\"\n";
			for (const std::filesystem::path& filepath : filepaths) {
				std::string relativeFilepath = std::filesystem::relative(filepath, outputDir).string();
				outCppFile << "#include \"" << relativeFilepath << "\"\n";
			}
			outCppFile << '\n';

			outCppFile << "extern \"C\" {\n";

			// GetComponents
			outCppFile << "\tDLL_EXPORT std::vector<Sphynx::Scripting::ComponentReflectionInfo>* GetComponents() {\n";
			outCppFile << "\t\tstatic std::vector<Sphynx::Scripting::ComponentReflectionInfo> s_Components = {\n";
			{
				for (const auto& component : components) {
					outCppFile << "\t\t\t{\n";
					outCppFile << "\t\t\t\t\"" << component.Fullname << "\",\n";
					outCppFile << "\t\t\t\t\"" << component.Filepath << "\",\n";
					outCppFile << "\t\t\t\t{\n";
					for (const auto& variable : component.Variables)
						outCppFile << "\t\t\t\t\t{\t\"" << variable.Type << "\", \"" << variable.Name << "\", offsetof(" << component.Fullname << ", " << variable.Name << ") },\n";
				
					outCppFile << "\t\t\t\t},\n";
					outCppFile << "\t\t\t\tsizeof(" << component.Fullname << "),\n";
					outCppFile << "\t\t\t\t[](void* _this) { std::construct_at(reinterpret_cast<" << component.Fullname << "*>(_this)); },\n";
					outCppFile << "\t\t\t\t[](void* _this) { std::destroy_at(reinterpret_cast<" << component.Fullname << "*>(_this)); },\n";
					outCppFile << "\t\t\t\t[](void* _this, void* other) { std::construct_at(reinterpret_cast<" << component.Fullname << "*>(_this), *reinterpret_cast<" << component.Fullname << "*>(other)); }\n";
					outCppFile << "\t\t\t},\n";
				}
			}
			outCppFile << "\t\t};\n";
			outCppFile << "\t\treturn &s_Components;\n";
			outCppFile << "\t}\n";


			// GetConfigs
//			outCppFile << "\tDLL_EXPORT std::vector<Sphynx::Scripting::ConfigReflectionInfo>* GetConfigs() {\n";
//			outCppFile << "\t\tstatic std::vector<Sphynx::Scripting::ConfigReflectionInfo> s_Configs = {\n";
//			{
//				size_t nextTypeID = 0;
//				for (const auto& config : configs) {
//					outCppFile << "\t\t\t{\n";
//					outCppFile << "\t\t\t\t" << nextTypeID << ",\n";
//					outCppFile << "\t\t\t\t\"" << config.Fullname << "\",\n";
//					outCppFile << "\t\t\t\t{\n";
//					for (const auto& variable : config.Variables) {
//						outCppFile << "\t\t\t\t\t{\n";
//						outCppFile << "\t\t\t\t\t\t\"" << variable.Type << "\", \"" << variable.Name << "\", offsetof(" << config.Fullname << ", " << variable.Name << "), sizeof(decltype(" << config.Fullname << "::" << variable.Name << ")),\n";
//						outCppFile << "\t\t\t\t\t\t[](void* ptr, void* other) { std::construct_at(reinterpret_cast<decltype(" << config.Fullname << "::" << variable.Name << ")*>(ptr), *reinterpret_cast<decltype(" << config.Fullname << "::" << variable.Name << ")*>(other)); }\n";
//						outCppFile << "\t\t\t\t\t},\n";
//					}
//					outCppFile << "\t\t\t\t},\n";
//					outCppFile << "\t\t\t\tsizeof(" << config.Fullname << "),\n";
//					outCppFile << "\t\t\t\t[](void* ptr) { std::construct_at(reinterpret_cast<" << config.Fullname << "*>(ptr)); },\n";
//					outCppFile << "\t\t\t\t[](void* ptr) { std::destroy_at(reinterpret_cast<" << config.Fullname << "*>(ptr)); },\n";
//					outCppFile << "\t\t\t\t[](void* ptr, void* other) { std::construct_at(reinterpret_cast<" << config.Fullname << "*>(ptr), *reinterpret_cast<" << config.Fullname << "*>(other)); }\n";
//					outCppFile << "\t\t\t},\n";
//
//					nextTypeID++;
//				}
//			}
//			outCppFile << "\t\t};\n";
//			outCppFile << "\t\treturn &s_Configs;\n";
//			outCppFile << "\t}\n";
//
//			// GetSystems
//			outCppFile << "\tDLL_EXPORT std::vector<Sphynx::Scripting::SystemReflectionInfo>* GetSystems() {\n";
//			outCppFile << "\t\tstatic std::vector<Sphynx::Scripting::SystemReflectionInfo> s_Systems = {\n";
//			{
//				size_t nextTypeID = 0;
//				for (const auto& system : systems) {
//					outCppFile << "\t\t\t{\n";
//					outCppFile << "\t\t\t\t" << nextTypeID << ",\n";
//					outCppFile << "\t\t\t\t\"" << system.Fullname << "\",\n";
//					outCppFile << "\t\t\t\t" << system.Fullname <<  "\n";
//					outCppFile << "\t\t\t},\n";
//					nextTypeID++;
//				}
//			}
//			outCppFile << "\t\t};\n";
//			outCppFile << "\t\treturn &s_Systems;\n";
//			outCppFile << "\t}\n";

			// IsDebugConfiguration
			outCppFile << "\tDLL_EXPORT bool IsDebugConfiguration() {\n";
			outCppFile << "#ifdef DEBUG\n";
			outCppFile << "\t\treturn true;\n";
			outCppFile << "#else\n";
			outCppFile << "\t\treturn false;\n";
			outCppFile << "#endif\n";
			outCppFile << "\t}\n";
			
			outCppFile << "}\n";

			const std::filesystem::path cacheFilepath = outputDir / "generated.cache";
			if (!CachedReflectionInfo::Serialize(cacheFilepath, components, filepaths, srcDir))
				std::cout << "Failed to generate cache file in " << cacheFilepath.string() << '\n';
		}
	};
}