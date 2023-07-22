#pragma once

#include "CrashReportWindow.hpp"

#include <iostream>

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

namespace Sphynx {
	GLFWwindow* s_Window = nullptr;
	CrashReporterWindow* s_CrashReporterWindow = nullptr;

	static void Fatal(const char* msg) {
		MessageBoxA(glfwGetWin32Window(s_Window), msg, "CrashReporter - Fatal", 0);

		std::exit(1);
	}

	static void CrashReporterGUIUpdate() {
		glClearColor(0.1f, 0.1f, 0.1f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();


		if (!(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable))
			return;

		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		static bool opened = true;
		ImGui::Begin("DockingSpace", &opened, window_flags);

		ImGui::PopStyleVar();
		ImGui::PopStyleVar(2);

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			ImGuiID dockspace_id = ImGui::GetID("DockingSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0, 0), dockspace_flags);
		}
				

		ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.1f, 0.1f, 0.1f, 1.f });
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Exit"))
					glfwSetWindowShouldClose(s_Window, true);
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
		ImGui::PopStyleColor();

		s_CrashReporterWindow->Draw();

		if (!(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable))
			return;

		ImGui::End();



		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		int display_w, display_h;
		glfwGetFramebufferSize(s_Window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);

		glfwSwapBuffers(s_Window);
		glfwPollEvents();
	}
	
	static void glfw_error_callback(int error, const char* description) {
		std::cout << "GLFW Error " << error << ": " << description << '\n';
	}

	static void glfw_resize_callback(GLFWwindow* /*window*/, int width, int height) {
		glViewport(0, 0, width, height);

		CrashReporterGUIUpdate();
	}

	static void CrashReporterGUIRun() {
		glfwSetErrorCallback(glfw_error_callback);

		if (glfwInit() == GLFW_FALSE)
			Fatal("Failed to initialize glfw");
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_SAMPLES, 4);

		s_Window = glfwCreateWindow(1200, 800, "CrashReporter", nullptr, nullptr);
		if (!s_Window)
			Fatal("Failed to create window");

		glfwMakeContextCurrent(s_Window);

		if (gladLoadGL() == 0)
			Fatal("Failed to initialize glad");

		glViewport(0, 0, 1400, 800);

		glfwSetWindowSizeCallback(s_Window, glfw_resize_callback);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.FontDefault = io.Fonts->AddFontFromFileTTF("Fonts/OpenSans/static/OpenSans-Regular.ttf", 18.f);
		io.IniFilename = "imgui.ini";

		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForOpenGL(s_Window, true);
		ImGui_ImplOpenGL3_Init("#version 330");

		s_CrashReporterWindow = new CrashReporterWindow();

		while (!glfwWindowShouldClose(s_Window)) {
			CrashReporterGUIUpdate();			
		}

		delete s_CrashReporterWindow;

		// Cleanup
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		glfwDestroyWindow(s_Window);
		glfwTerminate();
	}
}