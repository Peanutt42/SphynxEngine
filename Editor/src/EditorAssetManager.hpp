#pragma once

#include "EditorApplication.hpp"

#include "Importers/MeshImporter.hpp"
#include "Importers/ShaderImporter.hpp"

namespace Sphynx::Editor {
	class EditorAssetManager {
	public:
		static void LoadAssets() {
			const std::filesystem::path assetPath = Engine::GetProject()->Folderpath / "Assets";
			for (const auto& directoryEntry : std::filesystem::recursive_directory_iterator(assetPath)) {
				const std::filesystem::path path = directoryEntry.path();
				const std::filesystem::path fileExtension = path.extension();

				if (fileExtension == ".obj" ||
					fileExtension == ".fbx" ||
					fileExtension == ".gltf")
				{
					std::filesystem::path convertedFilepath = path;
					convertedFilepath.replace_extension(".semesh");
					if (!std::filesystem::exists(convertedFilepath)) {
						Rendering::MeshData data;
						MeshImporter::Import(path, data);
						data.SaveMesh(convertedFilepath);
					}
				}

				if (fileExtension == ".glsl") {
					std::vector<uint32_t> vertexCode, fragmentCode;
					ShaderImporter::Import(path, vertexCode, fragmentCode);
				}
			}
		}
	};
}