#pragma once

#include "pch.hpp"

namespace Sphynx::Rendering {
	class SE_API Shader {
	public:
		Shader(const std::filesystem::path& vertex_filepath, const std::filesystem::path& fragment_filepath);
		~Shader();

		void Bind();

		void Set(std::string_view name, bool value);
		void Set(std::string_view name, int value);
		void Set(std::string_view name, float value);

	private:
		std::optional<int> GetUniformLocation(std::string_view name);

	private:
		uint32 m_ProgramID = 0;
		std::unordered_map<std::string_view, int> m_UniformlocationMap;
	};
}