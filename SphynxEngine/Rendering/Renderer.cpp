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

	struct Vertex {
		glm::vec3 position;

		static VertexLayout GetVertexLayout() {
			return VertexLayout{}
			.add(VertexAttrib::Vec3);
		}
	};

	struct Billboard {
		glm::vec3 Position;
		uint32 TextureID = 0;
	};

	struct RenderCommand {
		std::vector<glm::mat4> ModelMatrices;
		std::vector<Billboard> Billboards;
		std::vector<Vertex> Lines;
		Camera SceneCamera;
	};
	RenderCommand s_RenderCommand;

	Framebuffer* s_SceneFramebuffer = nullptr;
	
	Shader* default_shader = nullptr;
	Shader* line_shader = nullptr;
	Shader* billboard_shader = nullptr;
	Mesh* quad = nullptr;

	Texture* cat = nullptr;

	Mesh* cube = nullptr;
	std::vector<Vertex> cube_vertices = {
		{{-0.5f, -0.5f,  0.5f}},
		{{ 0.5f, -0.5f,  0.5f}},
		{{-0.5f,  0.5f,  0.5f}},
		{{ 0.5f,  0.5f,  0.5f}},
		{{-0.5f, -0.5f, -0.5f}},
		{{ 0.5f, -0.5f, -0.5f}},
		{{-0.5f,  0.5f, -0.5f}},
		{{ 0.5f,  0.5f, -0.5f}},
	};
	std::vector<uint32> cube_indices = {
		2, 6, 7, 2, 3, 7, 0, 4, 5, 0, 1, 5,
		0, 2, 6, 0, 4, 6, 1, 3, 7, 1, 5, 7,
		0, 2, 3, 0, 1, 3, 4, 6, 7, 4, 5, 7
	};

	struct BillboardVertex {
		glm::vec3 Position;
		glm::vec2 UV;

		static VertexLayout GetVertexLayout() {
			return VertexLayout{}
				.add(VertexAttrib::Vec3)
				.add(VertexAttrib::Vec2);
		}
	};
	std::vector<BillboardVertex> quad_vertices = {
		{{-0.5f, 0.5f, 0.f}, {0.f, 1.f}},
		{{ 0.5f, 0.5f, 0.f}, {1.f, 1.f}},
		{{ 0.5f,-0.5f, 0.f}, {1.f,  0.f}},
		{{-0.5f,-0.5f, 0.f}, {0.f,  0.f}},
	};
	std::vector<uint32> quad_indices = { 0,1,2,2,3,0 };

	std::shared_ptr<VertexArray> s_BatchLineVA;
	std::shared_ptr<VertexBuffer> s_BatchLineVB;
	constexpr uint32 k_MaxLines = 1024;

	int s_ScreenWidth = 0, s_ScreenHeight = 0;
	constexpr int s_SceneWidth = 1920, s_SceneHeight = 1080;

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

		s_SceneFramebuffer = new Framebuffer(s_SceneWidth, s_SceneHeight);

		default_shader = new Shader("Content/Shaders/Default.vert", "Content/Shaders/Default.frag");
		default_shader->Bind();
		cat = new Texture("Content/Textures/cat.jpg");

		billboard_shader = new Shader("Content/Shaders/Billboard.vert", "Content/Shaders/Billboard.frag");
		billboard_shader->Bind();

		line_shader = new Shader("Content/Shaders/Line.vert", "Content/Shaders/Line.frag");

		s_BatchLineVA = std::make_shared<VertexArray>();
		s_BatchLineVB = std::make_shared<VertexBuffer>(k_MaxLines, Vertex::GetVertexLayout());
		s_BatchLineVA->AddVertexBuffer(s_BatchLineVB);
		s_RenderCommand.Lines.reserve(k_MaxLines);

		quad = new Mesh(quad_vertices, quad_indices);

		cube = new Mesh(cube_vertices, cube_indices);
		
		s_Initialized = true;
		return true;
	}

	void Renderer::Shutdown() {
		SE_PROFILE_FUNCTION();

		if (!s_Initialized)
			return;

		delete cube;
		delete cat;
		delete default_shader;
		delete line_shader;
		delete billboard_shader;
		delete quad;

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

	void Renderer::SubmitBillboard(const glm::vec3& position, uint32 textureID) {
		s_RenderCommand.Billboards.emplace_back(position, textureID);
	}

	void Renderer::SubmitLine(const glm::vec3& start, const glm::vec3& end) {
		s_RenderCommand.Lines.emplace_back(start);
		s_RenderCommand.Lines.emplace_back(end);
	}

	void Renderer::Update() {
		SE_PROFILE_FUNCTION();

		if (!s_Initialized)
			return;

		s_SceneFramebuffer->Bind();

		default_shader->Bind();
		float aspect = GetAspect(s_SceneWidth, s_SceneHeight);
		auto proj_view = s_RenderCommand.SceneCamera.GetPerspective(aspect) * s_RenderCommand.SceneCamera.GetView();
		default_shader->Set("proj_view", proj_view);
		for (const auto& modelMatrix : s_RenderCommand.ModelMatrices) {
			default_shader->Set("model_matrix", modelMatrix);
			cube->Draw();
		}

		glDisable(GL_DEPTH_TEST);

		// billboards
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		billboard_shader->Bind();
		billboard_shader->Set("billboard", 0);
		billboard_shader->Set("proj_view", proj_view);
		for (const auto& billboard : s_RenderCommand.Billboards) {
			Texture::Bind(billboard.TextureID, 0);

			glm::mat4 model_matrix = glm::translate(glm::mat4(1.f), billboard.Position) * 
				glm::toMat4(glm::quat(s_RenderCommand.SceneCamera.Rotation));
			billboard_shader->Set("model_matrix", model_matrix);
			quad->Draw();
		}
		s_RenderCommand.Billboards.resize(0);
		glDisable(GL_BLEND);

		// Lines
		line_shader->Bind();
		line_shader->Set("mvp", proj_view);
		line_shader->Set("color", glm::vec3(0, 1, 0));
		glLineWidth(2.5f);
		s_BatchLineVB->SetData(s_RenderCommand.Lines);
		s_BatchLineVA->Bind();
		glDrawArrays(GL_LINES, 0, s_RenderCommand.Lines.size());
		s_RenderCommand.Lines.resize(0);

		glEnable(GL_DEPTH_TEST);

		// default framebuffer
		s_SceneFramebuffer->Unbind();
		glViewport(0, 0, s_ScreenWidth, s_ScreenHeight);
		glClearColor(0.2f, 0.3f, 0.3f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	uint32 Renderer::GetSceneTextureID() { return s_SceneFramebuffer->GetColorTextureID(); }

	bool Renderer::IsInitialized() { return s_Initialized; }
}