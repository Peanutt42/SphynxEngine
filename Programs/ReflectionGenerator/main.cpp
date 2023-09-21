#include "Tokenizer.hpp"
#include "Parser.hpp"
#include "Generator.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <chrono>

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <debugapi.h>
#endif

void remove_char(std::string& str, char c) {
    std::string tmp;
    tmp.reserve(str.size());
    for (char srcChar : str) {
        if (srcChar != c)
            tmp.push_back(srcChar);
    }
    str = tmp;
}

int main(int argc, const char** argv) {
	// If not run inside IDE -> run inside working dir
	// else -> Run tests
#ifdef WINDOWS
	if (!IsDebuggerPresent()) {
#endif
    if (argc != 4) {
        std::cout << "usage: [Input] [Output] [EngineDir]\n";
        std::cin.get();
        return 1;
    }

    std::string engineDir = argv[3];
    remove_char(engineDir, '\"');

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<Sphynx::Scripting::ComponentReflectionInfo> components;
    std::vector<Sphynx::Scripting::ConfigReflectionInfo> configs;
    std::vector<Sphynx::Scripting::SystemReflectionInfo> systems;
    std::vector<std::string> filenames;

    std::string inputDirStr = argv[1];
    remove_char(inputDirStr, '\"');

	std::filesystem::path inputDir = inputDirStr;
	if (!std::filesystem::exists(inputDir)) {
		std::cout << "Input \"" << inputDirStr << "\" doesn't exist!" << std::endl;
		return 1;
	}

    for (const auto& directoryEntry : std::filesystem::recursive_directory_iterator(inputDir)) {
        const std::filesystem::path& path = directoryEntry.path();

        std::string extension = path.extension().string();
        if (extension != ".hpp" && extension != ".h" && extension != ".cpp")
            continue;

        std::ifstream file(path, std::ios::ate);
        if (file.is_open()) {
            size_t end = file.tellg();
            file.seekg(0, std::ios::beg);
            size_t fileSize = end - file.tellg();
            std::string code;
            code.resize(fileSize);
            file.read(code.data(), fileSize);

            std::filesystem::path relativePath = std::filesystem::relative(path, std::filesystem::current_path());

            Sphynx::Scripting::Tokenizer tokenizer(code);
            if (!tokenizer.IsSuccessful()) {
				std::cout << "[ReflectionGenerator] Tokenizer failed: " << tokenizer.GetErrorMessage() << '\n';
				return 1;
			}

			size_t nextComponentIndex = components.size();
			size_t nextConfigIndex = configs.size();
			size_t nextSystemIndex = systems.size();
            Sphynx::Scripting::Parser parser(tokenizer.GetTokens(), relativePath.string(), components, configs, systems);
            if (!parser.IsSuccessful()) {
				std::cout << "[ReflectionGenerator] Parser failed: " << parser.GetErrorMessagee() << '\n';
				return 1;
			}
            filenames.push_back(path.string());


			for (size_t i = nextComponentIndex; i < components.size(); i++)
				std::cout << components[i].FullName << std::endl;
			for (size_t i = nextConfigIndex; i < configs.size(); i++)
				std::cout << configs[i].FullName << std::endl;
			for (size_t i = nextSystemIndex; i < systems.size(); i++)
				std::cout << systems[i].FullName << std::endl;
        }
    }


	std::string outputPathStr = argv[2];
	remove_char(outputPathStr, '\"');
	std::filesystem::path outputPath = outputPathStr;
    Sphynx::Scripting::Generator::Generate(components, configs, systems, filenames, outputPath, engineDir);

    std::cout << "[ReflectionGenerator] Finished in " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() << "ms -> " << outputPathStr << std::endl;
#ifdef WINDOWS
	}
	else {
		std::vector<Sphynx::Scripting::ComponentReflectionInfo> components;
		std::vector<Sphynx::Scripting::ConfigReflectionInfo> configs;
		std::vector<Sphynx::Scripting::SystemReflectionInfo> systems;
		std::vector<std::string> filenames;

		std::filesystem::path filepath = "Programs/ReflectionGenerator/src/TestFile.hpp";
		std::string extension = filepath.extension().string();

		std::ifstream file(filepath, std::ios::ate);
		if (file.is_open()) {
			size_t end = file.tellg();
			file.seekg(0, std::ios::beg);
			size_t fileSize = end - file.tellg();
			std::string code;
			code.resize(fileSize);
			file.read(code.data(), fileSize);

			filenames.push_back(filepath.filename().string());
			std::filesystem::path relativePath = std::filesystem::relative(filepath, std::filesystem::current_path());

			Sphynx::Scripting::Tokenizer tokenizer(code);
			if (!tokenizer.IsSuccessful())
				std::cout << "[ReflectionGenerator] Tokenizer failed: " << tokenizer.GetErrorMessage() << '\n';

			Sphynx::Scripting::Parser parser(tokenizer.GetTokens(), relativePath.string(), components, configs, systems);
			if (!parser.IsSuccessful())
				std::cout << "[ReflectionGenerator] Parser failed: " << parser.GetErrorMessagee() << '\n';
		}
		std::cout << "Components:\n";
		for (const auto& component : components) {
			std::cout << "\t" << component.FullName << '\n';
			for (const auto& var : component.Variables)
				std::cout << "\t\t" << var.Type << " " << var.Name << '\n';
		}
		std::cout << "Configs:\n";
		for (const auto& config : configs) {
			std::cout << "\t" << config.FullName << '\n';
			for (const auto& var : config.Variables)
				std::cout << "\t\t" << var.Type << " " << var.Name << '\n';
		}
		std::cout << "Systems:\n";
		for (const auto& system : systems)
			std::cout << "\t" << system.FullName << '\n';

		std::cout << "Finished\n";
	}
#endif
}