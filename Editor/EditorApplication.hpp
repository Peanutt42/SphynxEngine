#pragma once

#include "pch.hpp"
#include "Core/Engine.hpp"
#include "UI/VulkanImGui.hpp"
#include "EditorWindow.hpp"
#include "Scene/Scene.hpp"
#include "Rendering/Camera.hpp"

namespace Sphynx::Editor {
	enum class EditorState {
		Editing,
		Playing
	};

	struct ECSGameSystemInfo {
		bool Active = true;
		float LastDeltatime = 0.f;
	};

	class EditorApplication : public Application {
	public:
		virtual void OnCreate() override;
		virtual void OnDestroy() override;

		virtual void Update() override;
		virtual void DrawUI() override;
		void OnDrawMenubar();

		void OnRuntimeStart();
		void OnRuntimeUpdate();
		void OnRuntimeStop();

		static Scene& GetEditingScene() { return *s_Instance->m_EditingScene; }
		static Scene& GetGameScene() { return *s_Instance->m_GameScene; }
		static Scene& GetCurrentScene() { return s_Instance->m_State == EditorState::Editing ? GetEditingScene() : GetGameScene(); }
		static void SetSceneDirty(bool dirty) { s_Instance->m_SceneDirty = dirty; }

		static void SetECSSystemActive(const std::string& name, bool active);
		static const ECSGameSystemInfo* GetECSSystem(const std::string& name) {
			auto find = s_Instance->m_GameECSSystems.find(name);
			if (find == s_Instance->m_GameECSSystems.end())
				return nullptr;
			return &find->second;
		}
		static float GetECSSystemDeltaTime() { return s_Instance->m_GameTotalECSSystemDeltaTime; }

		static EditorState GetState() { return s_Instance->m_State; }
		static Rendering::Camera& GetEditingCamera() { return s_Instance->m_EditingCamera; }

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
		Rendering::Camera m_EditingCamera;
		bool m_SceneDirty = false;
		std::unique_ptr<Scene> m_GameScene;
		float m_GameTotalECSSystemDeltaTime = 0.f;
		std::unordered_map<std::string, ECSGameSystemInfo> m_GameECSSystems;
	};
}