#include "pch.hpp"
#include "EditorApplication.hpp"
#include "Physics/PhysicEngine.hpp"
#include "Profiling/Profiling.hpp"
#include "Rendering/RenderingComponents.hpp"

#include "Scene/SceneSerializer.hpp"

#include "Windows/LoggingOutputWindow.hpp"
#include "Windows/HierarchyWindow.hpp"
#include "Windows/PropertyWindow.hpp"
#include "Windows/ProfilingWindow.hpp"
#include "Windows/ViewportWindow.hpp"

#include "Guizmos.hpp"

namespace Sphynx::Editor {
	void EditorApplication::OnCreate() {
		SE_PROFILE_FUNCTION();

		s_Instance = this;

		ImGui::SetCurrentContext(UI::GetContext());

		m_Windows.push_back(std::make_unique<LoggingOutputWindow>());
		m_Windows.push_back(std::make_unique<HierarchyWindow>());
		m_Windows.push_back(std::make_unique<PropertyWindow>());
		m_Windows.push_back(std::make_unique<ProfilingWindow>());
		m_Windows.push_back(std::make_unique<ViewportWindow>());

		m_EditingScene = std::make_unique<Scene>("Empty");
		m_SceneFilepath = Engine::GetProject().StartSceneFilepath;
		SceneSerializer::Deserialize(m_SceneFilepath, *m_EditingScene)
			.expect("Failed to load start scene {}", m_SceneFilepath.string());

		Guizmos::Init();
	}

	void EditorApplication::OnDestroy() {
		SE_PROFILE_FUNCTION();
	
		Guizmos::Shutdown();

		m_Windows.clear();
		m_EditingScene.reset();
		m_GameScene.reset();

		s_Instance = nullptr;
	}
	
	void EditorApplication::Update() {
		SE_PROFILE_FUNCTION();

		for (const auto& window : m_Windows)
			window->Update();

		if (Input::IsKeyDown(KeyCode::LeftControl)) {
			if (Input::IsKeyPressed(KeyCode::N))
				CreateNewScene();

			if (Input::IsKeyPressed(KeyCode::S)) {
				if (Input::IsKeyDown(KeyCode::LeftShift))
					SaveScene();
				else
					SaveCurrentScene();
			}
			if (Input::IsKeyPressed(KeyCode::O))
				OpenScene();
		}

		if (Input::IsKeyPressed(KeyCode::F5))
			OnRuntimeStart();
		if (Input::IsKeyPressed(KeyCode::F8))
			OnRuntimeStop();
		
		if (m_State == EditorState::Playing)
			OnRuntimeUpdate();

		Rendering::Renderer::SubmitScene(m_State == EditorState::Editing ? *m_EditingScene : *m_GameScene, m_State == EditorState::Editing ? m_EditingCamera : Rendering::Camera{}); // TODO: find active camera in game scene
		if (m_State == EditorState::Editing) {
			for (auto [entity, transform, light] : m_EditingScene->View<ECS::TransformComponent, Rendering::LightComponent>().each()) {
				Rendering::Renderer::SubmitBillboard(transform.Position, Guizmos::s_LightBulb->GetID());
			}
			for (auto [entity, transform, camera] : m_EditingScene->View<ECS::TransformComponent, Rendering::CameraComponent>().each()) {
				Rendering::Renderer::SubmitBillboard(transform.Position, Guizmos::s_Camera->GetID());
			}
			Rendering::Renderer::SubmitLine(glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f));
		}
	}

	void EditorApplication::DrawUI() {
		SE_PROFILE_FUNCTION();

		for (const auto& window : m_Windows) {
			if (!window->Opened)
				continue;

			if (ImGui::Begin(window->Name.c_str(), &window->Opened))
				window->Draw();
			ImGui::End();
		}

		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New Scene", "Ctrl + N"))
					CreateNewScene();

				if (ImGui::MenuItem("Open Scene", "Ctrl + O"))
					OpenScene();

				if (ImGui::MenuItem("Save Scene", "Ctrl + S"))
					SaveCurrentScene();

				if (ImGui::MenuItem("Save Scene to File", "Ctrl + Shift + S"))
					SaveScene();

				if (ImGui::MenuItem("Exit", "Alt + F4"))
					Engine::CloseNextFrame();

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Windows")) {
				for (auto& window : m_Windows) {
					if (ImGui::MenuItem(window->Name.c_str()))
						window->Opened = true;
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	void EditorApplication::OnDrawMenubar() {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New Scene", "Ctrl + N"))
				CreateNewScene();

			if (ImGui::MenuItem("Open Scene", "Ctrl + O"))
				OpenScene();

			if (ImGui::MenuItem("Save Scene", "Ctrl + S"))
				SaveCurrentScene();

			if (ImGui::MenuItem("Save Scene to File", "Ctrl + Shift + S"))
				SaveScene();

			if (ImGui::MenuItem("Exit", "Alt + F4"))
				Engine::CloseNextFrame();

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Windows")) {
			for (auto& window : m_Windows) {
				if (ImGui::MenuItem(window->Name.c_str()))
					window->Opened = true;
			}

			ImGui::EndMenu();
		}
	}

	void EditorApplication::OnRuntimeStart() {
		m_State = EditorState::Playing;
		m_GameScene = std::make_unique<Scene>(*m_EditingScene);
		Physics::PhysicEngine::ClearWorld();
	}

	void EditorApplication::OnRuntimeUpdate() {
		// Update Physics
		Physics::PhysicEngine::Update(*m_GameScene);
	}

	void EditorApplication::OnRuntimeStop() {
		m_GameScene.reset();
		m_State = EditorState::Editing;
	}


	void EditorApplication::CreateNewScene() {
		if (m_SceneDirty)
			SaveScene();

		m_EditingScene = std::make_unique<Scene>("New");
		m_SceneDirty = true;
		m_SceneFilepath.clear();
	}

	void EditorApplication::OpenScene() {
		if (m_SceneDirty && Platform::MessagePrompts::YesNo("Unsaved Scene", "Do you want to save your changes?"))
			SaveScene();
		
		m_SceneFilepath = Platform::FileDialogs::OpenFile("Sphynx Engine Scene", "*.sescene");

		if (!std::filesystem::exists(m_SceneFilepath))
			return;

		m_EditingScene = std::make_unique<Scene>("Empty");
		SceneSerializer::Deserialize(m_SceneFilepath, *m_EditingScene)
			.expect("Failed to open scene {}", m_SceneFilepath.string());
	}

	void EditorApplication::SaveCurrentScene() {
		if (!std::filesystem::exists(m_SceneFilepath))
			m_SceneFilepath = Platform::FileDialogs::SaveFile("Sphynx Engine Scene", "*.sescene");
		
		SceneSerializer::Serialize(m_SceneFilepath, *m_EditingScene);
		m_SceneDirty = false;
	}

	void EditorApplication::SaveScene() {
		m_SceneFilepath = Platform::FileDialogs::SaveFile("Sphynx Engine Scene", "*.sescene");
		SceneSerializer::Serialize(m_SceneFilepath, *m_EditingScene);
		m_SceneDirty = false;
	}
}