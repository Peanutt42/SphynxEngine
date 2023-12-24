#pragma once

#include "EditorApplication.hpp"
#include "Rendering/Mesh.hpp"

namespace Sphynx::Editor {
	class MeshImporter {
	public:
		static Result<bool, std::string> Import(const std::filesystem::path& filepath, Rendering::MeshData& outData);
	};
}