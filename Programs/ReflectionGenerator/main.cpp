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
    if (argc != 1) {
		std::cout << "TODO";
    }


    // run tests
    else {
		auto start_time = std::chrono::high_resolution_clock::now();

	    std::vector<Sphynx::ReflectionGenerator::ComponentReflectionInfo> components;
	    std::vector<std::filesystem::path> filepaths;
		std::vector<std::filesystem::path> changedFilepaths;
		const std::filesystem::path srcDir = "../Tests/src";

	    Sphynx::ReflectionGenerator::CachedReflectionInfo::LoadUnchanged("../Tests/generated/generated.cache", srcDir, filepaths, components, changedFilepaths);

		size_t changedFilesCount = 0;
		for (const auto& filepath : changedFilepaths) {
			std::ifstream file(filepath, std::ios::ate);
			if (file.is_open()) {
				std::cout << "Compiling " << std::filesystem::relative(filepath, srcDir).string() << '\n';
				
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

	    Sphynx::ReflectionGenerator::Generator::Generate(components, filepaths, srcDir, "../Tests/generated", "../../../../");

		std::cout << "[ReflectionGenerator] Finished in " << std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start_time) << std::endl;
    }
}