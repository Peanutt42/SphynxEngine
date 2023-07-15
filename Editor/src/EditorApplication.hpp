#pragma once

#include "Core/CoreInclude.hpp"
#include "Core/Engine.hpp"
#include "UI/VulkanImGuiHelper.hpp"
#include "EditorWindow.hpp"

namespace Sphynx::Editor {
	class EditorApplication : public Application {
	public:
		virtual void OnCreate() override;
		virtual void OnDestroy() override;

		virtual void Update() override;
		virtual void DrawUI() override;

	private:
		std::vector<std::unique_ptr<EditorWindow>> m_Windows;
	};
}