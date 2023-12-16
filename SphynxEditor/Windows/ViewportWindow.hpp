#pragma once

#include "EditorWindow.hpp"
#include "Rendering/Renderer.hpp"
#include "Input/Input.hpp"

namespace Sphynx::Editor {
	class ViewportWindow : public EditorWindow {
	public:
		ViewportWindow() {
			Name = "Viewport";
		}

		virtual void Update() override {
			if (m_Hovered && EditorApplication::GetState() == EditorState::Editing && Input::IsMouseButtonPressed(MouseButton::Right))
				m_Looking = true;
			if (Input::IsMouseButtonReleased(MouseButton::Right))
				m_Looking = false;
			
			if (!m_Looking)
				return;

			float movementSpeed = 2.5f;
			float mouseSensitivity = 0.002f;

			glm::vec2 mouseMovement = Input::GetMouseDelta();
			Rendering::Camera& editingCamera = EditorApplication::GetEditingCamera();
			editingCamera.Rotation.y -= mouseMovement.x * mouseSensitivity;
			editingCamera.Rotation.x -= mouseMovement.y * mouseSensitivity;
			editingCamera.Rotation.x = std::clamp(editingCamera.Rotation.x, glm::radians(-89.99999f), glm::radians(89.99999f));

			glm::vec3 inputDir = { 0,0,0 };
			if (Input::IsKeyDown(KeyCode::A)) inputDir.x++;
			if (Input::IsKeyDown(KeyCode::D)) inputDir.x--;
			if (Input::IsKeyDown(KeyCode::W)) inputDir.z++;
			if (Input::IsKeyDown(KeyCode::S)) inputDir.z--;
			if (Input::IsKeyDown(KeyCode::E)) inputDir.y++;
			if (Input::IsKeyDown(KeyCode::Q)) inputDir.y--;
			float inputDirLength = glm::length(inputDir);
			if (inputDirLength <= 0.01f)
				return;
			
			if (inputDirLength > 1.f)
				inputDir = glm::normalize(inputDir);
			
			float moveSpeed = movementSpeed * glm::length(inputDir) * Engine::DeltaTime();
			if (Input::IsKeyDown(KeyCode::LeftShift)) moveSpeed *= 2;
			if (Input::IsKeyDown(KeyCode::LeftControl)) moveSpeed /= 2;

			glm::vec3 forward = editingCamera.GetForward();
			glm::vec3 right = editingCamera.GetRight();
			glm::vec3 up = editingCamera.GetUp();
			glm::vec3 moveDir{ 0,0,0 };
			moveDir += inputDir.z * forward;
			moveDir += inputDir.x * -right;
			moveDir += inputDir.y * up;
			moveDir = glm::normalize(moveDir);
			editingCamera.Position += moveDir * moveSpeed;
		}

		virtual void Draw() override {
			m_Hovered = ImGui::IsWindowHovered();

		}

	private:
		bool m_Hovered = false;
		bool m_Looking = false;
	};
}