#include "pch.hpp"
#include "Renderer.hpp"
#include "Scene/DefaultComponents.hpp"
#include "Profiling/Profiling.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include "Texture.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Sphynx::Rendering {
	bool s_Initialized = false;

	struct RenderCommand {
		Camera SceneCamera;
	};
	RenderCommand s_RenderCommand;

	Shader* triangle_shader = nullptr;

	Texture* cat = nullptr;

	Mesh* triangle = nullptr;
	struct Vertex {
		glm::vec3 position, color;
		glm::vec2 uv;

		static VertexLayout GetVertexLayout() {
			return VertexLayout{}
			.add(VertexAttrib::Vec3)
			.add(VertexAttrib::Vec3)
			.add(VertexAttrib::Vec2);
		}
	};
	std::vector<Vertex> vertices = {
		// positions          // colors           // texture coords
		{{ 0.5f,  0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},   // top right
		{{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},   // bottom right
		{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},   // bottom left
		{{-0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}    // top left 
	};
	std::vector<uint32> indices = {
		0, 1, 3,
		1, 2, 3
	};

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

		triangle_shader = new Shader("Content/Shaders/triangle.vert", "Content/Shaders/triangle.frag");
		triangle_shader->Bind();
		triangle_shader->Set("a_texture", 0);
		cat = new Texture("Content/Textures/cat.jpg");

		triangle = new Mesh(vertices, indices);
		
		s_Initialized = true;
		return true;
	}

	void Renderer::Shutdown() {
		SE_PROFILE_FUNCTION();

		if (!s_Initialized)
			return;

		delete triangle;
		delete cat;
		delete triangle_shader;

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
		
		triangle_shader->Bind();
		cat->Bind();
		triangle->Draw();
	}

	bool Renderer::IsInitialized() { return s_Initialized; }
}