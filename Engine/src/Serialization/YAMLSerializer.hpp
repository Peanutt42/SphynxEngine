#pragma once

#include "Core/CoreInclude.hpp"
#include <yaml-cpp/yaml.h>

namespace Sphynx::Serialization {
	const bool SaveYamlToFile(const std::filesystem::path& filepath, const YAML::Emitter& emitter) {
		std::ofstream fout(filepath);
		if (!fout) {
			//PE_ERR(Logging::General, "Failed to save file '{}'", filepath.string());
			return false;
		}

		fout << emitter.c_str();
		fout.close();
		return true;
	}
}