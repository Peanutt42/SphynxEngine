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
        return 0;
    }

    std::string engineDir = argv[3];
    remove_char(engineDir, '\"');

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<Sphynx::Scripting::ComponentReflectionInfo> components;
    std::vector<Sphynx::Scripting::ConfigReflectionInfo> configs;
    std::vector<Sphynx::Scripting::SystemReflectionInfo> systems;
    std::vector<std::string> filenames;

    std::string inputDir = argv[1];
    remove_char(inputDir, '\"');
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
            if (!tokenizer.IsSuccessful())
                std::cout << "[ReflectionGenerator] Tokenizer failed: " << tokenizer.GetErrorMessage() << '\n';
            Sphynx::Scripting::Parser parser(tokenizer.GetTokens(), relativePath.string(), components, configs, systems);
            if (!parser.IsSuccessful())
                std::cout << "[ReflectionGenerator] Parser failed: " << parser.GetErrorMessagee() << '\n';
            if (extension == ".hpp" || extension == ".h")
                filenames.push_back(path.string());
        }
    }

    std::string generatedFolderStr = argv[2];
    remove_char(generatedFolderStr, '\"');
    std::cout << std::filesystem::current_path().string() << "/" << generatedFolderStr << std::endl;
    std::filesystem::path generatedFolder = generatedFolderStr;
    if (!std::filesystem::exists(generatedFolder))
        std::filesystem::create_directory(generatedFolder);

    Sphynx::Scripting::Generator::Generate(components, configs, systems, filenames, generatedFolder / "Module.cpp", engineDir);

    std::cout << "[ReflectionGenerator] Finished in " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() << "ms\n";
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