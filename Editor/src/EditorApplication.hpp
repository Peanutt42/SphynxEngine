#pragma once

#include "Core/CoreInclude.hpp"
#include "Core/Engine.hpp"
#include "UI/VulkanImGuiHelper.hpp"
#include "EditorWindow.hpp"
#include "Scene/Scene.hpp"

namespace Sphynx::Editor {
	enum class EditorState {
		Editing,
		Playing
	};

	class EditorApplication : public Application {
	public:
		EditorApplication();
		~EditorApplication();

		virtual void OnCreate() override;
		virtual void OnDestroy() override;

		virtual void Update() override;
		virtual void DrawUI() override;

		static Scene& GetEditingScene() { return *s_Instance->m_EditingScene; }
		static Scene& GetGameScene() { return *s_Instance->m_GameScene; }
		static Scene& GetCurrentScene() { return s_Instance->m_State == EditorState::Editing ? GetEditingScene() : GetGameScene(); }

		void CreateNewScene();
		void OpenScene();
		void SaveCurrentScene();
		void SaveScene();

	private:
		inline static EditorApplication* s_Instance = nullptr;

		std::vector<std::unique_ptr<EditorWindow>> m_Windows;

		EditorState m_State = EditorState::Editing;
		std::unique_ptr<Scene> m_EditingScene;
		std::filesystem::path m_SceneFilepath;
		bool m_SceneDirty = false;
		bool m_GameRunning = false;
		std::unique_ptr<Scene> m_GameScene;
	};
}