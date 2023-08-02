#pragma once

#include "EditorApplication.hpp"

namespace Sphynx::Editor {
	class ShaderImporter {
	public:
		static void Import(const std::filesystem::path& filepath, std::vector<uint32_t>& outVertexCode, std::vector<uint32_t>& outFragmentCode);
	};
}