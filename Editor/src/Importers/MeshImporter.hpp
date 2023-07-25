#pragma once

#include "EditorApplication.hpp"
#include "Rendering/Mesh.hpp"

namespace Sphynx::Editor {
	class MeshImporter {
	public:
		static void Import(const std::filesystem::path& filepath, Rendering::MeshData& outData);
	};
}