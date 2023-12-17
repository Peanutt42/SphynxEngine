#include "pch.hpp"
#include "Shader.hpp"
#include "Serialization/FileStream.hpp"

#include <glad/glad.h>

namespace Sphynx::Rendering {
	std::optional<uint32> CompileShader(const std::filesystem::path& filepath) {
		GLenum type = filepath.filename().string().ends_with(".vert") ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;
		uint32 shader = glCreateShader(type);
		std::string shaderCode;
		if (FileStreamReader::ReadTextFile(filepath, shaderCode).is_error() || shaderCode.empty())
			return std::nullopt;
		const char* shaderCodePtr = shaderCode.data();
		glShaderSource(shader, 1, &shaderCodePtr, nullptr);
		glCompileShader(shader);
		int success = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
			SE_WARN(Logging::Rendering, "Failed to compile {}: {}", filepath.string(), infoLog);
			return std::nullopt;
		}
		return shader;
	}

	Shader::Shader(const std::filesystem::path& vertex_filepath, const std::filesystem::path& fragment_filepath) {
		auto vertexShader = CompileShader(vertex_filepath);
		auto fragmentShader = CompileShader(fragment_filepath);
		if (!vertexShader || !fragmentShader) {
			SE_ERR(Logging::Rendering, "Can't compile this shader!");
			return;
		}

		m_ProgramID = glCreateProgram();
		glAttachShader(m_ProgramID, *vertexShader);
		glAttachShader(m_ProgramID, *fragmentShader);
		glLinkProgram(m_ProgramID);
		int success = 0;
		glGetProgramiv(m_ProgramID, GL_LINK_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetProgramInfoLog(m_ProgramID, sizeof(infoLog), nullptr, infoLog);
			SE_ERR(Logging::Rendering, "Failed to compile shader ({}, {}): {}", vertex_filepath.string(), fragment_filepath.string(), infoLog);
			return;
		}
		glDeleteShader(*vertexShader);
		glDeleteShader(*fragmentShader);
	}

	Shader::~Shader() {
		glDeleteProgram(m_ProgramID);
	}

	void Shader::Bind() {
		glUseProgram(m_ProgramID);
	}

	void Shader::Set(std::string_view name, bool value) {
		if (auto location = GetUniformLocation(name))
			glUniform1i(*location, (int)value);
	}

	void Shader::Set(std::string_view name, int value) {
		if (auto location = GetUniformLocation(name))
			glUniform1i(*location, value);
	}

	void Shader::Set(std::string_view name, float value) {
		if (auto location = GetUniformLocation(name))
			glUniform1f(*location, value);
	}

	std::optional<int> Shader::GetUniformLocation(std::string_view name) {
		auto find = m_UniformlocationMap.find(name);
		if (find != m_UniformlocationMap.end())
			return find->second;
		int location = glGetUniformLocation(m_ProgramID, name.data());
		if (location == -1)
			return std::nullopt;
		m_UniformlocationMap[name] = location;
		return location;
	}
}