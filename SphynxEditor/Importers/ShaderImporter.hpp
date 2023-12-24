#pragma once

#include "EditorApplication.hpp"

namespace Sphynx::Editor {
	class ShaderImporter {
	public:
		static Result<bool, std::string> Import(const std::filesystem::path& filepath, std::vector<uint32>& outVertexCode, std::vector<uint32>& outFragmentCode);
	};
}