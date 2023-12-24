#pragma once

#include "EditorWindow.hpp"
#include "Core/Engine.hpp"
#include "Core/Time.hpp"

namespace Sphynx::Editor {
	class ProfilingWindow : public EditorWindow {
	public:
		ProfilingWindow() {
			Name = "Profiling";
		}

		virtual void Draw() override {
			if (m_OneSecondTimer.ElapsedSeconds() >= 1.f) {
				m_OneSecondTimer.Reset();
				m_CurrentAvgFps = m_FrameCount;
				m_FrameCount = 0;
			}
			else
				m_FrameCount++;

			ImGui::Text("FPS: %f.3\n\t%f.5 ms", 1.f / Engine::DeltaTime(), Engine::DeltaTime() * 1000.f);
			ImGui::Text("Avg FPS: %d", m_CurrentAvgFps);

			ImGuiIO& io = ImGui::GetIO();
			ImGui::Text("Editor UI: %d triangles", io.MetricsRenderIndices / 3);
		}

	private:
		int m_CurrentAvgFps = 0;
		int m_FrameCount = 0;
		Timer m_OneSecondTimer;
	};
}