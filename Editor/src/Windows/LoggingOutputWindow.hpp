#pragma once

#include "EditorWindow.hpp"

namespace Sphynx::Editor {
	class LoggingOutputWindow : public EditorWindow {
	public:
		LoggingOutputWindow() {
			Name = "Logging Output";
			if (!s_LogCallbackAttatched) {
				Logging::RegisterOnLogCallback([](Logging::Verbosity verbosity, Logging::Category category, const std::string& msg) {
					s_Logs.emplace_back(msg, verbosity, category);
				});
				s_Logs.reserve(1024);
				s_LogCallbackAttatched = true;
			}
			SE_INFO(Logging::Editor, "This is a test");
			SE_WARN(Logging::Editor, "This is a test");
			SE_ERR(Logging::Editor, "This is a test");
		}

		virtual void Draw() override {
			if (ImGui::Button("Clear")) {
				s_Logs.clear();
				s_Logs.reserve(1024);
			}

			if (ImGui::BeginChild("scrolling", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar)) {
				ImGuiListClipper clipper;
				clipper.Begin((int)s_Logs.size());
				while (clipper.Step()) {
					for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
						switch (s_Logs[i].Verbosity) {
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
						ImGui::TextUnformatted(Logging::CategoryToString(s_Logs[i].Category));
						ImGui::SameLine();
						ImGui::TextUnformatted(s_Logs[i].Msg.c_str());
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
		inline static std::vector<LogEntry> s_Logs;
		inline static bool s_LogCallbackAttatched = false;
	};
}