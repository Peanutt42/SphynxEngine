#include "Tokenizer.hpp"
#include "Parser.hpp"
#include "Generator.hpp"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

int main(const int argc, const char** argv) {
    // run as tool
    if (argc == 3) {
		std::filesystem::path sourceDir = argv[1];
		if (!std::filesystem::exists(sourceDir)) {
			std::cout << "Can't find '" << sourceDir.string() << "'\n";
			return 1;
		}

		std::filesystem::path outputDir = argv[2];
		if (!std::filesystem::exists(outputDir)) {
			std::cout << "Can't find '" << outputDir.string() << "'\n";
			return 1;
		}

		auto start_time = std::chrono::high_resolution_clock::now();

		std::vector<Sphynx::ReflectionGenerator::ComponentReflectionInfo> components;
		std::vector<std::filesystem::path> filepaths;
		std::vector<std::filesystem::path> changedFilepaths;

		Sphynx::ReflectionGenerator::CachedReflectionInfo::LoadUnchanged(outputDir /"generated.cache", sourceDir, filepaths, components, changedFilepaths);

		size_t changedFilesCount = 0;
		for (const auto& filepath : changedFilepaths) {
			std::ifstream file(filepath, std::ios::ate);
			if (file.is_open()) {
				std::cout << "Compiling " << std::filesystem::relative(filepath, sourceDir).string() << '\n';

				size_t end = file.tellg();
				file.seekg(0, std::ios::beg);
				size_t fileSize = end - file.tellg();
				std::string code;
				code.resize(fileSize);
				file.read(code.data(), fileSize);

				Sphynx::ReflectionGenerator::Tokenizer tokenizer(code);
				if (!tokenizer.IsSuccessful())
					std::cout << "\t[ReflectionGenerator] Tokenizer failed: " << tokenizer.GetErrorMessage() << '\n';

				Sphynx::ReflectionGenerator::Parser parser(tokenizer.GetTokens(), filepath.string(), components);
				if (!parser.IsSuccessful())
					std::cout << "\t[ReflectionGenerator] Parser failed: " << parser.GetErrorMessagee() << '\n';

				changedFilesCount++;
			}
		}

		std::cout << "[ReflectionGenerator] " << changedFilesCount << " files changed\n";

		Sphynx::ReflectionGenerator::Generator::Generate(components, filepaths, sourceDir, outputDir);

		std::cout << "[ReflectionGenerator] Finished in " << std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start_time).count() << " seconds\n";
    }


    // run tests
    else {
		auto start_time = std::chrono::high_resolution_clock::now();

	    std::vector<Sphynx::ReflectionGenerator::ComponentReflectionInfo> components;
	    std::vector<std::filesystem::path> filepaths;
		std::vector<std::filesystem::path> changedFilepaths;
		const std::filesystem::path sourceDir = "../Tests/src";

	    Sphynx::ReflectionGenerator::CachedReflectionInfo::LoadUnchanged("../Tests/generated/generated.cache", sourceDir, filepaths, components, changedFilepaths);

		size_t changedFilesCount = 0;
		for (const auto& filepath : changedFilepaths) {
			std::ifstream file(filepath, std::ios::ate);
			if (file.is_open()) {
				std::cout << "Compiling " << std::filesystem::relative(filepath, sourceDir).string() << '\n';
				
				size_t end = file.tellg();
				file.seekg(0, std::ios::beg);
				size_t fileSize = end - file.tellg();
				std::string code;
				code.resize(fileSize);
				file.read(code.data(), fileSize);

				Sphynx::ReflectionGenerator::Tokenizer tokenizer(code);
				if (!tokenizer.IsSuccessful())
					std::cout << "\t[ReflectionGenerator] Tokenizer failed: " << tokenizer.GetErrorMessage() << '\n';

				Sphynx::ReflectionGenerator::Parser parser(tokenizer.GetTokens(), filepath.string(), components);
				if (!parser.IsSuccessful())
					std::cout << "\t[ReflectionGenerator] Parser failed: " << parser.GetErrorMessagee() << '\n';

				changedFilesCount++;
			}
		}

		std::cout << changedFilesCount << " files changed\n";

	    std::cout << "Components:\n";
	    for (const auto& component : components) {
		    std::cout << "\t" << component.Fullname << '\n';
		    for (const auto& var : component.Variables)
			    std::cout << "\t\t" << var.Type << " " << var.Name << '\n';
	    }

	    Sphynx::ReflectionGenerator::Generator::Generate(components, filepaths, sourceDir, "../Tests/generated");

		std::cout << "[ReflectionGenerator] Finished in " << std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start_time).count() << " seconds\n";
    }
}