#include "pch.hpp"
#include "Renderer.hpp"
#include "Scene/DefaultComponents.hpp"
#include "Profiling/Profiling.hpp"
#include "Serialization/FileStream.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Sphynx::Rendering {
	bool s_Initialized = false;

	struct RenderCommand {
		Camera SceneCamera;
	};
	RenderCommand s_RenderCommand;

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

	std::optional<uint32> CompileProgram(const std::filesystem::path& vertex, const std::filesystem::path& fragment) {
		auto vertexShader = CompileShader(vertex);
		auto fragmentShader = CompileShader(fragment);
		if (!vertexShader || !fragmentShader)
			return std::nullopt;

		uint32 program = glCreateProgram();
		glAttachShader(program, *vertexShader);
		glAttachShader(program, *fragmentShader);
		glLinkProgram(program);
		int success = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
			SE_WARN(Logging::Rendering, "Failed to compile shader ({}, {}): {}", vertex.string(), fragment.string(), infoLog);
			return std::nullopt;
		}
		glDeleteShader(*vertexShader);
		glDeleteShader(*fragmentShader);
		return program;
	}

	uint32 vbo, vao, ebo;
	float vertices[] = {
	 0.5f,  0.5f, 0.0f,  // top right
	 0.5f, -0.5f, 0.0f,  // bottom right
	-0.5f, -0.5f, 0.0f,  // bottom left
	-0.5f,  0.5f, 0.0f   // top left 
	};
	unsigned int indices[] = {  // note that we start from 0!
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};
	uint32 program;

	bool Renderer::Init(Window& window, const std::function<void()>& resizeCallback) {
		SE_PROFILE_FUNCTION();

		if (s_Initialized)
			return true;

		window.SetResizeCallback([resizeCallback](Window* window, int width, int height) {
			if (width != 0 && height != 0) {
				glViewport(0, 0, width, height);
				if (resizeCallback)
					resizeCallback();
			}
		});

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			SE_ERR(Logging::Rendering, "Failed to initialize glad!");
			return false;
		}

		glViewport(0, 0, window.GetWidth(), window.GetHeight());

		program = *CompileProgram("Content/Shaders/triangle.vert", "Content/Shaders/triangle.frag");

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		s_Initialized = true;
		return true;
	}

	void Renderer::Shutdown() {
		SE_PROFILE_FUNCTION();

		if (!s_Initialized)
			return;

		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		glDeleteProgram(program);

		s_Initialized = false;
	}

	void Renderer::SubmitScene(Scene& scene, const Camera& camera) {
		if (!s_Initialized)
			return;

		s_RenderCommand.SceneCamera = camera;
	}

	void Renderer::Update() {
		SE_PROFILE_FUNCTION();

		if (!s_Initialized)
			return;

		glClearColor(0.2f, 0.3f, 0.3f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(program);
		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	bool Renderer::IsInitialized() { return s_Initialized; }
}