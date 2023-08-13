#pragma once

#include <yaml-cpp/yaml.h>
#include "../../../Engine/src/Debug/StackTrace.hpp"
#include <filesystem>
#include <iostream>
#include <imgui.h>

namespace Sphynx {
	struct CrashData {
		struct SystemInfo {
			std::string OS;
			std::string Architecture;
		} SystemInfo;
		
		struct CrashInfo {
			std::string Process;
			std::string Thread;
			std::string Reason;
			std::vector<std::string> LastErrorMessages;
		} CrashInfo;
		
		StackTrace StackTrace;
	};
	
	class CrashReporterWindow {
	public:
		CrashReporterWindow() {
			if (!std::filesystem::exists("CrashReport.txt")) {
				std::cout << "Couldn't find CrashReport.txt!\n";
				return;
			}
			
			try {
				YAML::Node data = YAML::LoadFile("CrashReport.txt");
				m_CrashData.SystemInfo.OS = data["OS"].as<std::string>();
				m_CrashData.SystemInfo.Architecture = data["Archetecture"].as<std::string>();
				auto crashNode = data["Crash"];
				if (crashNode) {
					m_CrashData.CrashInfo.Process = crashNode["Process"].as<std::string>();
					m_CrashData.CrashInfo.Thread = crashNode["Thread"].as<std::string>();
					m_CrashData.CrashInfo.Reason = crashNode["Reason"].as<std::string>();
					auto lastErrorMsgsNode = crashNode["LastErrorMessages"];
					if (lastErrorMsgsNode) {
						for (const YAML::Node& msg : lastErrorMsgsNode) {
							m_CrashData.CrashInfo.LastErrorMessages.push_back(msg.as<std::string>());
						}
					}
				}
				auto stackTraceNode = data["Stacktrace"];
				for (auto stackTraceEntryNode : stackTraceNode) {
					auto& entry = m_CrashData.StackTrace.emplace_back();
					entry.FunctionName = stackTraceEntryNode["FunctionName"].as<std::string>();
					auto sourceFileNode = stackTraceEntryNode["SourceFile"];
					if (sourceFileNode)
						entry.SourceFile = sourceFileNode.as<std::string>();
					auto sourceLineNode = stackTraceEntryNode["SourceLine"];
					if (sourceLineNode)
						entry.SourceLine = sourceLineNode.as<size_t>();
					entry.HasSource = sourceFileNode || sourceLineNode;
				}


				m_CouldOpenCrashData = true;
			}
			catch (const std::exception& e) {
				std::cout << "Failed to parse crash data! " << e.what() << '\n';
			}
		}
		
		void Draw() {
			ImGui::Begin("Crash Report");
			
			if (m_CouldOpenCrashData) {
				ImGui::Text("System");
				ImGui::BulletText("OS: %s", m_CrashData.SystemInfo.OS.c_str());
				ImGui::BulletText("Architecture: %s", m_CrashData.SystemInfo.Architecture.c_str());

				ImGui::Text("Crash");
				ImGui::BulletText("Process: %s", m_CrashData.CrashInfo.Process.c_str());
				ImGui::BulletText("Thread: %s", m_CrashData.CrashInfo.Thread.c_str());
				ImGui::BulletText("Reason: %s", m_CrashData.CrashInfo.Reason.c_str());
				ImGui::BulletText("Last Error messages:");
				for (const std::string& msg : m_CrashData.CrashInfo.LastErrorMessages)
					ImGui::BulletText("\t%s", msg.c_str());

				ImGui::Text("StackTrace");
				static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Hideable | ImGuiTableFlags_NoSavedSettings;
				if (ImGui::BeginTable("StackTrace", 3, flags, { 0.f, ImGui::GetContentRegionAvail().y / 1.5f })) {
					ImGui::TableSetupColumn("Index");
					ImGui::TableSetupColumn("Function");
					ImGui::TableSetupColumn("Source");
					ImGui::TableHeadersRow();
					for (size_t i = 0; i < m_CrashData.StackTrace.size(); i++) {
						auto& entry = m_CrashData.StackTrace[i];
						static ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
						int id = ImGui::GetID(("##" + std::to_string(i)).c_str());
						ImGui::PushID(id);
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("%d", i + 1);
						ImGui::TableSetColumnIndex(1);
						static size_t s_Selection;
						if (ImGui::Selectable(entry.FunctionName.c_str(), s_Selection == i, selectable_flags))
							s_Selection = i;

						if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right))
							ImGui::OpenPopup("StackTraceEntry Menu");

						ImGui::TableSetColumnIndex(2);
						ImGui::Text("%s", entry.HasSource ? (entry.SourceFile + ":" + std::to_string(entry.SourceLine)).c_str() : "???");

						if (ImGui::BeginPopup("StackTraceEntry Menu", ImGuiWindowFlags_NoMove)) {
							if (m_CrashData.StackTrace[i].HasSource) {
								if (ImGui::MenuItem("Open file"))
									system(m_CrashData.StackTrace[i].SourceFile.c_str());

								if (ImGui::MenuItem("Open in VSCode"))
									system(("code -r -g " + m_CrashData.StackTrace[i].SourceFile + ":" + std::to_string(m_CrashData.StackTrace[i].SourceLine)).c_str());
							}
							ImGui::EndPopup();
						}
						ImGui::PopID();
					}
					ImGui::EndTable();
				}
			}
			else {
				if (ImGui::BeginPopupModal("Failed to open the crash data")) {
					ImGui::TextUnformatted("Couldn't open crash data,\n"
						"it should have been in Programs/CrashReporter/CrashReport.txt!");

					ImGui::EndPopup();
				}
				ImGui::OpenPopup("Failed to open the crash data");
			}
			
			ImGui::End();
		}
		
	private:
		CrashData m_CrashData;
		bool m_CouldOpenCrashData = false;
	};
}