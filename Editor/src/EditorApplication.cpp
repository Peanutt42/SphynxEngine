#include "pch.hpp"
#include "EditorApplication.hpp"

#include "Windows/LoggingOutputWindow.hpp"

namespace Sphynx::Editor {
	void EditorApplication::OnCreate() {
		Engine::ImGuiHelper().SetContextOnDiffrentDll();

		Engine::ImGuiHelper().EnableDocking();
		Engine::ImGuiHelper().SetSaveFilepath("Engine/imgui.ini");

		m_Windows.push_back(std::make_unique<LoggingOutputWindow>());
	}

	void EditorApplication::OnDestroy() {
		m_Windows.clear();
	}
	
	void EditorApplication::Update() {
		for (const auto& window : m_Windows)
			window->Update();
	}

	void EditorApplication::DrawUI() {
		Engine::ImGuiHelper().BeginDockspace("Dockspace");

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

		Engine::ImGuiHelper().EndDockspace();
	}
}