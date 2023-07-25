#pragma once

#include "EditorApplication.hpp"

#include "Importers/MeshImporter.hpp"

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
					Rendering::MeshData data;
					MeshImporter::Import(path, data);
					std::filesystem::path convertedFilepath = path;
					convertedFilepath.replace_extension(".semesh");
					data.SaveMesh(convertedFilepath);
				}
			}
		}
	};
}