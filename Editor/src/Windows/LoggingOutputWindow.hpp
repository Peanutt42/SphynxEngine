#pragma once

#include "EditorWindow.hpp"

namespace Sphynx::Editor {
	class LoggingOutputWindow : public EditorWindow {
	public:
		LoggingOutputWindow() {
			Name = "Logging Output";
			Logging::RegisterOnLogCallback([this](Logging::Verbosity verbosity, Logging::Category category, const std::string& msg) {
				if (this)
					m_Logs.emplace_back(msg, verbosity, category);
			});
			m_Logs.reserve(1024);
			SE_INFO(Logging::Editor, "This is a test");
			SE_WARN(Logging::Editor, "This is a test");
			SE_ERR(Logging::Editor, "This is a test");
		}

		virtual void Draw() override {
			if (ImGui::Button("Clear")) {
				m_Logs.clear();
				m_Logs.reserve(1024);
			}

			if (ImGui::BeginChild("scrolling", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar)) {
				ImGuiListClipper clipper;
				clipper.Begin((int)m_Logs.size());
				while (clipper.Step()) {
					for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
						switch (m_Logs[i].Verbosity) {
						case Logging::Verbosity::Trace:
						case Logging::Verbosity::Info:
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.f, 1.f, 1.f, 1.f});
							break;
						case Logging::Verbosity::Warning:
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.f, 0.5f, 0.f, 1.f});
							break;
						case Logging::Verbosity::Error:
						case Logging::Verbosity::Critical:
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.f, 0.f, 0.f, 1.f});
							break;
						}
						ImGui::TextUnformatted(Logging::CategoryToString(m_Logs[i].Category));
						ImGui::SameLine();
						ImGui::TextUnformatted(m_Logs[i].Msg.c_str());
						ImGui::PopStyleColor();
					}
				}
				clipper.End();

				if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
					ImGui::SetScrollHereY(1.0f);

				ImGui::EndChild();
			}
		}

	private:
		struct LogEntry {
			std::string Msg;
			Logging::Verbosity Verbosity;
			Logging::Category Category;
		};
		std::vector<LogEntry> m_Logs;
	};
}