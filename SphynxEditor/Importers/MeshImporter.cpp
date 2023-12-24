#include "pch.hpp"
#include "MeshImporter.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Sphynx::Editor {
	Result<bool, std::string> MeshImporter::Import(const std::filesystem::path& filepath, Rendering::MeshData& outData) {
		std::string filepathStr = filepath.string();

		auto extension = filepath.extension();

		if (extension == L".pemesh") {
			SE_WARN(Logging::Rendering, "Model importer should be used to import various 3d model file formats into .pemesh format.\nJust use modelData.LoadPEMesh(...)");
			outData.LoadMesh(filepath);
		}
		else {
			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(filepathStr, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes);

			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
				return Error<bool>("Failed to load 3d model file '{}' with assimp: {}", filepathStr, importer.GetErrorString());
			else {
				auto processNode = [&](aiNode* node) {
					uint32 indexOffset = 0;
					for (unsigned int i = 0; i < node->mNumMeshes; i++) {
						aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
						for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
							Rendering::Vertex vertex;
							vertex.Position = {
								mesh->mVertices[v].x,
								mesh->mVertices[v].y,
								mesh->mVertices[v].z
							};

							vertex.Normal = {
								mesh->mNormals[v].x,
								mesh->mNormals[v].y,
								mesh->mNormals[v].z
							};

							if (mesh->mTextureCoords[0]) {
								vertex.UV = {
									mesh->mTextureCoords[0][v].x,
									mesh->mTextureCoords[0][v].y
								};
							}
							else
								vertex.UV = { 0.f, 0.f };

							outData.Vertices.push_back(vertex);
						}
						for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
							const aiFace& face = mesh->mFaces[f];
							for (unsigned int j = 0; j < face.mNumIndices; j++)
								outData.Indices.push_back(indexOffset + face.mIndices[j]);
						}
						indexOffset += (uint32)outData.Indices.back();
					}
					};
				processNode(scene->mRootNode);
				for (unsigned int i = 0; i < scene->mRootNode->mNumChildren; i++)
					processNode(scene->mRootNode->mChildren[i]);

				importer.FreeScene();

				// TODO: Maybe?
				std::filesystem::path peMeshFilepath(filepath);
				peMeshFilepath.replace_extension(L".semesh");
				outData.SaveMesh(peMeshFilepath);
			}
		}

		return true;
	}
}