#include "pch.hpp"
#include "Renderer.hpp"
#include "Scene/DefaultComponents.hpp"
#include "Profiling/Profiling.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include "Texture.hpp"
#include "Framebuffer.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Sphynx::Rendering {
	bool s_Initialized = false;

	struct RenderCommand {
		std::vector<glm::mat4> ModelMatrices;
		Camera SceneCamera;
	};
	RenderCommand s_RenderCommand;

	Framebuffer* s_SceneFramebuffer = nullptr;
	
	Shader* triangle_shader = nullptr;

	Texture* cat = nullptr;

	Mesh* triangle = nullptr;
	struct Vertex {
		glm::vec3 position;

		static VertexLayout GetVertexLayout() {
			return VertexLayout{}
			.add(VertexAttrib::Vec3);
		}
	};
	std::vector<Vertex> vertices = {
		{{-1, -1,  0.5}},
		{{ 1, -1,  0.5}},
		{{-1,  1,  0.5}},
		{{ 1,  1,  0.5}},
		{{-1, -1, -0.5}},
		{{ 1, -1, -0.5}},
		{{-1,  1, -0.5}},
		{{ 1,  1, -0.5}},
	};
	std::vector<uint32> indices = {
		2, 6, 7, 2, 3, 7, 0, 4, 5, 0, 1, 5,
		0, 2, 6, 0, 4, 6, 1, 3, 7, 1, 5, 7,
		0, 2, 3, 0, 1, 3, 4, 6, 7, 4, 5, 7
	};

	int s_ScreenWidth = 0, s_ScreenHeight = 0;

	bool Renderer::Init(Window& window, const std::function<void()>& resizeCallback, bool vsync) {
		SE_PROFILE_FUNCTION();

		if (s_Initialized)
			return true;

		glfwSwapInterval(vsync ? 1 : 0);

		window.SetResizeCallback([resizeCallback](Window* window, int width, int height) {
			if (width != 0 && height != 0) {
				s_ScreenWidth = width;
				s_ScreenHeight = height;
				glViewport(0, 0, s_ScreenWidth, s_ScreenHeight);
				if (resizeCallback)
					resizeCallback();
			}
		});
		s_ScreenWidth = window.GetWidth();
		s_ScreenHeight = window.GetHeight();

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			SE_ERR(Logging::Rendering, "Failed to initialize glad!");
			return false;
		}

		glViewport(0, 0, window.GetWidth(), window.GetHeight());

		s_SceneFramebuffer = new Framebuffer(1920, 1080);

		triangle_shader = new Shader("Content/Shaders/triangle.vert", "Content/Shaders/triangle.frag");
		triangle_shader->Bind();
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

		delete s_SceneFramebuffer;

		s_Initialized = false;
	}

	void Renderer::SubmitScene(Scene& scene, const Camera& camera) {
		if (!s_Initialized)
			return;

		s_RenderCommand.SceneCamera = camera;
		s_RenderCommand.ModelMatrices.resize(0);
		for (auto[entity, transform] : scene.View<ECS::TransformComponent>().each()) {
			s_RenderCommand.ModelMatrices.push_back(transform.GetModelMatrix());
		}
	}

	void Renderer::Update() {
		SE_PROFILE_FUNCTION();

		if (!s_Initialized)
			return;

		s_SceneFramebuffer->Bind();

		triangle_shader->Bind();
		float aspect = s_ScreenWidth / s_ScreenHeight;
		if (std::isnan(aspect)) aspect = 16.f / 9.f;
		triangle_shader->Set("proj_view", s_RenderCommand.SceneCamera.GetPerspective(aspect) * s_RenderCommand.SceneCamera.GetView());
		for (const auto& modelMatrix : s_RenderCommand.ModelMatrices) {
			triangle_shader->Set("model_matrix", modelMatrix);
			triangle->Draw();
		}

		// default framebuffer
		Framebuffer::BindScreen();
		glViewport(0, 0, s_ScreenWidth, s_ScreenHeight);
		glClearColor(0.2f, 0.3f, 0.3f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	uint32 Renderer::GetSceneTextureID() { return s_SceneFramebuffer->GetColorTextureID(); }

	bool Renderer::IsInitialized() { return s_Initialized; }
}