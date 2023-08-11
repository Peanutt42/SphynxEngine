#include "pch.hpp"
#include "EditorApplication.hpp"
#include "EditorAssetManager.hpp"

#include "Scene/SceneSerializer.hpp"

#include "Windows/LoggingOutputWindow.hpp"
#include "Windows/HierarchyWindow.hpp"
#include "Windows/PropertyWindow.hpp"
#include "Windows/ProfilingWindow.hpp"
#include "Windows/ECSSystemManagerWindow.hpp"

namespace Sphynx::Editor {
	EditorApplication::EditorApplication() {
		s_Instance = this;
	}
	EditorApplication::~EditorApplication() {
		s_Instance = nullptr;
	}

	void EditorApplication::OnCreate() {
		SE_PROFILE_FUNCTION();

		ImGui::SetCurrentContext(Engine::ImGuiHelper().GetContext());

		Engine::ImGuiHelper().EnableDocking();
		Engine::ImGuiHelper().SetSaveFilepath("Editor/imgui.ini");
		Engine::ImGuiHelper().SetMenubarCallback([this]() { OnDrawMenubar(); });

		m_Windows.push_back(std::make_unique<LoggingOutputWindow>());
		m_Windows.push_back(std::make_unique<HierarchyWindow>());
		m_Windows.push_back(std::make_unique<PropertyWindow>());
		m_Windows.push_back(std::make_unique<ProfilingWindow>());
		m_Windows.push_back(std::make_unique<ECSSystemManagerWindow>());

		EditorAssetManager::LoadAssets();

		m_EditingScene = std::make_unique<Scene>("Empty");
		m_SceneFilepath = Engine::GetProject()->StartSceneFilepath;
		std::string loadSceneError;
		if (!SceneSerializer::Deserialize(m_SceneFilepath, *m_EditingScene, loadSceneError))
			SE_ERR(Logging::Editor, "Failed to load scene {}: {}", m_SceneFilepath.string(), loadSceneError);

		// TODO: Save it in a config
		// TODO: Do this also in code reloading (copy old, create new from updated systems
		//		 and set old settings to new map while keeping new systems active by default
		for (const auto& system : Engine::Scripting().GetSystems())
			m_GameECSSystemActiveMap[system.FullName] = true;
	}

	void EditorApplication::OnDestroy() {
		SE_PROFILE_FUNCTION();

		m_Windows.clear();
		m_EditingScene.reset();
		m_GameScene.reset();
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

		if (m_GameRunning) {
			UpdateGame();
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

	void EditorApplication::UpdateGame() {
		// Update ECS-Systems
		const std::vector<Scripting::SystemReflectionInfo>& systems = Engine::Scripting().GetSystems();
		for (const auto& [name, active] : m_GameECSSystemActiveMap) {
			if (!active)
				continue;

			auto findSystem = std::ranges::find_if(systems, [name](const auto& info) { return info.FullName == name; });
			if (findSystem == systems.end()) {
				SE_WARN(Logging::Scripting, "Can't find systems '{}'", name);
			}
			else {
				findSystem->Update(m_GameScene.get());
			}
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
		std::string loadDErrorMsg;
		if (!SceneSerializer::Deserialize(m_SceneFilepath, *m_EditingScene, loadDErrorMsg))
			SE_ERR(Logging::Editor, "Failed to open scene {}: {}", m_SceneFilepath.string(), loadDErrorMsg);
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