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
						auto result = MeshImporter::Import(path, data);
						if (result)
							data.SaveMesh(convertedFilepath);
						else
							SE_WARN(Logging::Editor, "Failed to load mesh: {}", result.get_error());
					}
				}

				if (fileExtension == ".glsl") {
					std::vector<uint32> vertexCode, fragmentCode;
					auto result = ShaderImporter::Import(path, vertexCode, fragmentCode);
					if (result.is_error())
						SE_WARN(Logging::Editor, "Failed to load shader: {}", result.get_error());
				}
			}
		}
	};
}