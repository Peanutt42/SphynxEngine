#pragma once

#include "Parser.hpp"
#include <filesystem>
#include <fstream>

namespace Sphynx::Scripting {
	class Generator {
	public:
		static void Generate(const std::vector<ComponentReflectionInfo>& components, const std::vector<ConfigReflectionInfo>& configs, const std::vector<SystemReflectionInfo>& systems, const std::vector<std::string>& filenames, const std::filesystem::path& filepath, const std::string& engineDir) {
			std::ofstream outFile(filepath);
			if (!outFile.is_open())
				return;

			outFile << "#include \"Core/CoreInclude.hpp\"\n";
			outFile << "#include \"" << engineDir << "/Programs/ReflectionGenerator/src/ReflectionInfo.hpp\"\n";
			outFile << "#include \"" << engineDir << "/Engine/src/Scene/Scene.hpp\"\n";
			for (const std::string& filename : filenames) {
				std::string relativeFilepath = std::filesystem::relative(filename, filepath.parent_path()).string();
				outFile << "#include \"" << relativeFilepath << "\"\n";
			}
			outFile << '\n';

			outFile << "extern \"C\" {\n";

			// GetComponents
			outFile << "\t__declspec(dllexport) std::vector<Sphynx::Scripting::ComponentReflectionInfo>* GetComponents() {\n";
			outFile << "\t\tstatic std::vector<Sphynx::Scripting::ComponentReflectionInfo> s_Components = {\n";
			{
				size_t nextTypeID = 0;
				for (const auto& component : components) {
					outFile << "\t\t\t{\n";
					outFile << "\t\t\t\t" << nextTypeID << ",\n";
					outFile << "\t\t\t\t\"" << component.FullName << "\",\n";
					outFile << "\t\t\t\t{\n";
					for (const auto& variable : component.Variables) {
						outFile << "\t\t\t\t\t{\n";
						outFile << "\t\t\t\t\t\t\"" << variable.Type << "\", \"" << variable.Name << "\", offsetof(" << component.FullName << ", " << variable.Name << "), sizeof(decltype(" << component.FullName << "::" << variable.Name << ")),\n";
						outFile << "\t\t\t\t\t\t[](void* ptr, void* other) { std::construct_at(reinterpret_cast<decltype(" << component.FullName << "::" << variable.Name << ")*>(ptr), *reinterpret_cast<decltype(" << component.FullName << "::" << variable.Name << ")*>(other)); }\n";
						outFile << "\t\t\t\t\t},\n";
					}
					outFile << "\t\t\t\t},\n";
					outFile << "\t\t\t\tsizeof(" << component.FullName << "),\n";
					outFile << "\t\t\t\t[](void* ptr) { std::construct_at(reinterpret_cast<" << component.FullName << "*>(ptr)); },\n";
					outFile << "\t\t\t\t[](void* ptr) { std::destroy_at(reinterpret_cast<" << component.FullName << "*>(ptr)); },\n";
					outFile << "\t\t\t\t[](void* ptr, void* other) { std::construct_at(reinterpret_cast<" << component.FullName << "*>(ptr), *reinterpret_cast<" << component.FullName << "*>(other)); }\n";
					outFile << "\t\t\t},\n";

					nextTypeID++;
				}
			}
			outFile << "\t\t};\n";
			outFile << "\t\treturn &s_Components;\n";
			outFile << "\t}\n";


			// GetConfigs
			outFile << "\t__declspec(dllexport) std::vector<Sphynx::Scripting::ConfigReflectionInfo>* GetConfigs() {\n";
			outFile << "\t\tstatic std::vector<Sphynx::Scripting::ConfigReflectionInfo> s_Configs = {\n";
			{
				size_t nextTypeID = 0;
				for (const auto& config : configs) {
					outFile << "\t\t\t{\n";
					outFile << "\t\t\t\t" << nextTypeID << ",\n";
					outFile << "\t\t\t\t\"" << config.FullName << "\",\n";
					outFile << "\t\t\t\t{\n";
					for (const auto& variable : config.Variables) {
						outFile << "\t\t\t\t\t{\n";
						outFile << "\t\t\t\t\t\t\"" << variable.Type << "\", \"" << variable.Name << "\", offsetof(" << config.FullName << ", " << variable.Name << "), sizeof(decltype(" << config.FullName << "::" << variable.Name << ")),\n";
						outFile << "\t\t\t\t\t\t[](void* ptr, void* other) { std::construct_at(reinterpret_cast<decltype(" << config.FullName << "::" << variable.Name << ")*>(ptr), *reinterpret_cast<decltype(" << config.FullName << "::" << variable.Name << ")*>(other)); }\n";
						outFile << "\t\t\t\t\t},\n";
					}
					outFile << "\t\t\t\t},\n";
					outFile << "\t\t\t\tsizeof(" << config.FullName << "),\n";
					outFile << "\t\t\t\t[](void* ptr) { std::construct_at(reinterpret_cast<" << config.FullName << "*>(ptr)); },\n";
					outFile << "\t\t\t\t[](void* ptr) { std::destroy_at(reinterpret_cast<" << config.FullName << "*>(ptr)); },\n";
					outFile << "\t\t\t\t[](void* ptr, void* other) { std::construct_at(reinterpret_cast<" << config.FullName << "*>(ptr), *reinterpret_cast<" << config.FullName << "*>(other)); }\n";
					outFile << "\t\t\t},\n";

					nextTypeID++;
				}
			}
			outFile << "\t\t};\n";
			outFile << "\t\treturn &s_Configs;\n";
			outFile << "\t}\n";

			// GetSystems
			outFile << "\t__declspec(dllexport) std::vector<Sphynx::Scripting::SystemReflectionInfo>* GetSystems() {\n";
			outFile << "\t\tstatic std::vector<Sphynx::Scripting::SystemReflectionInfo> s_Systems = {\n";
			{
				size_t nextTypeID = 0;
				for (const auto& system : systems) {
					outFile << "\t\t\t{\n";
					outFile << "\t\t\t\t" << nextTypeID << ",\n";
					outFile << "\t\t\t\t\"" << system.FullName << "\",\n";
					outFile << "\t\t\t\t[](void* scene) { " << system.FullName << "::Update(*(Sphynx::Scene*)scene); }\n";
					outFile << "\t\t\t},\n";
					nextTypeID++;
				}
			}
			outFile << "\t\t};\n";
			outFile << "\t\treturn &s_Systems;\n";
			outFile << "\t}\n";

			// IsDebugConfiguration
			outFile << "\t__declspec(dllexport) bool IsDebugConfiguration() {\n";
			outFile << "#ifdef DEBUG\n";
			outFile << "\t\treturn true;\n";
			outFile << "#else\n";
			outFile << "\t\treturn false;\n";
			outFile << "#endif\n";
			outFile << "\t}\n";
			
			outFile << "}\n";
		}
	};
}