#pragma once

#include "EditorWindow.hpp"
#include "Core/CommandHandler.hpp"

namespace Sphynx::Editor {
	class LoggingOutputWindow : public EditorWindow {
	public:
		LoggingOutputWindow() {
			Name = "Logging Output";
			if (!s_LogCallbackAttatched) {
				Logging::RegisterOnLogCallback([](Logging::Verbosity verbosity, Logging::Category category, const std::string& msg) {
					s_Logs.push_back(LogEntry{ msg, verbosity, category });
				});
				s_Logs.reserve(1024);
				s_LogCallbackAttatched = true;
			}
		}

		virtual void Draw() override {
			if (ImGui::Button("Clear")) {
				s_Logs.clear();
				s_Logs.reserve(1024);
			}
			
			constexpr int consoleInputHeight = 45;

			ImVec2 availableRegion = ImGui::GetContentRegionAvail();
			if (ImGui::BeginChild("scrolling", { availableRegion.x, availableRegion.y - consoleInputHeight }, true, ImGuiWindowFlags_HorizontalScrollbar)) {
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
			}
			ImGui::EndChild();

			static std::string s_Input;
			// TODO: ImGuiInputTextFlags_CallbackCompletion & ImGuiInputTextFlags_CallbackHistory
			//		 for input history and tab completion
			if (ImGui::InputText("##console input text", &s_Input, ImGuiInputTextFlags_EnterReturnsTrue)) {
				CommandHandler::QueueCommand(s_Input);
				s_Input.clear();
				ImGui::SetKeyboardFocusHere(-1);
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