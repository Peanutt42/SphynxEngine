#include "pch.hpp"
#include "EditorApplication.hpp"
#include "EditorAssetManager.hpp"

#include "Scene/SceneSerializer.hpp"

#include "Windows/LoggingOutputWindow.hpp"
#include "Windows/HierarchyWindow.hpp"
#include "Windows/PropertyWindow.hpp"
#include "Windows/ProfilingWindow.hpp"

namespace Sphynx::Editor {
	EditorApplication::EditorApplication() {
		s_Instance = this;
	}
	EditorApplication::~EditorApplication() {
		s_Instance = nullptr;
	}

	void EditorApplication::OnCreate() {
		ImGui::SetCurrentContext(Engine::ImGuiHelper().GetContext());

		Engine::ImGuiHelper().EnableDocking();
		Engine::ImGuiHelper().SetSaveFilepath("Engine/imgui.ini");
		Engine::ImGuiHelper().SetMenubarCallback([this]() { OnDrawMenubar(); });

		m_Windows.push_back(std::make_unique<LoggingOutputWindow>());
		m_Windows.push_back(std::make_unique<HierarchyWindow>());
		m_Windows.push_back(std::make_unique<PropertyWindow>());
		m_Windows.push_back(std::make_unique<ProfilingWindow>());

		EditorAssetManager::LoadAssets();

		m_EditingScene = std::make_unique<Scene>("Empty");
		m_SceneFilepath = Engine::GetProject()->StartSceneFilepath;
		SceneSerializer::Deserialize(m_SceneFilepath, *m_EditingScene);
	}

	void EditorApplication::OnDestroy() {
		m_Windows.clear();
	}
	
	void EditorApplication::Update() {
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
	}

	void EditorApplication::DrawUI() {
		for (const auto& window : m_Windows) {
			if (!window->Opened)
				continue;

			if (ImGui::Begin(window->Name.c_str(), &window->Opened))
				window->Draw();
			ImGui::End();
		}

		ImGui::Begin("Test");

		ImGui::Text("This is a test");

		ImGui::End();
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
		SceneSerializer::Deserialize(m_SceneFilepath, *m_EditingScene);
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