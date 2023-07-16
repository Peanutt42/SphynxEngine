#include "pch.hpp"
#include "Input.hpp"
#include "Profiling/Profiling.hpp"

#include <GLFW/glfw3.h>

namespace Sphynx {
	void Input::Init(GLFWwindow* window) {
		s_Window = window;
	}
	
	void Input::Update() {
		SE_PROFILE_FUNCTION();

		for (int i = MIN_KEY; i < KEY_COUNT; i++) {
			ButtonState state = (ButtonState)glfwGetKey(s_Window, i);
			s_KeyStateChanged[i] = s_KeyStates[i] != state;
			s_KeyStates[i] = state;
		}
		for (int i = 0; i < MOUSEBUTTON_COUNT; i++) {
			bool state = glfwGetMouseButton(s_Window, i) == GLFW_PRESS;
			s_MouseButtonStateChanged[i] = s_MouseButtonStates[i] != state;
			s_MouseButtonStates[i] = state;
		}

		double mouse_x, mouse_y;
		glfwGetCursorPos(s_Window, &mouse_x, &mouse_y);
		glm::vec2 currentMousePos = { (float)mouse_x, (float)mouse_y };
		s_LastMousePosDelta = currentMousePos - s_LastMousePos;
		s_LastMousePos = currentMousePos;
	}


	const bool Input::IsKeyUp(KeyCode key) {
		return GetKey(key) == ButtonState::Released;
	}

	const bool Input::IsKeyDown(KeyCode key) {
		ButtonState state = GetKey(key);
		return state == ButtonState::Pressed || state == ButtonState::Held;
	}

	const bool Input::IsKeyPressed(KeyCode key) {
		return s_KeyStateChanged[(uint16_t)key] && GetKey(key) == ButtonState::Pressed;
	}

	const bool Input::IsKeyReleased(KeyCode key) {
		return s_KeyStateChanged[(uint16_t)key] && GetKey(key) == ButtonState::Released;
	}

	const ButtonState Input::GetKey(KeyCode key) {
		return s_KeyStates[(uint16_t)key];
	}


	const bool Input::IsMouseButtonUp(MouseButton button) {
		return !s_MouseButtonStates[(uint16_t)button];
	}

	const bool Input::IsMouseButtonDown(MouseButton button) {
		return s_MouseButtonStates[(uint16_t)button];
	}

	const bool Input::IsMouseButtonPressed(MouseButton button) {
		return s_MouseButtonStateChanged[(uint16_t)button] && IsMouseButtonDown(button);
	}

	const bool Input::IsMouseButtonReleased(MouseButton button) {
		return s_MouseButtonStateChanged[(uint16_t)button] && IsMouseButtonUp(button);
	}


	void Input::SetMousePos(const glm::vec2& newPos) {
		glfwSetCursorPos(s_Window, newPos.x, newPos.y);
		s_LastMousePos = newPos;
	}

	const glm::vec2& Input::GetMousePos() { return s_LastMousePos; }

	const glm::vec2& Input::GetMouseDelta() { return s_LastMousePosDelta; }

	void Input::SetCursorMode(CursorMode mode) {
		glfwSetInputMode(s_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL + (int)mode);
	}
}