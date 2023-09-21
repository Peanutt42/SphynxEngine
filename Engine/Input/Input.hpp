#pragma once

#include "std.hpp"
#include "Math/Transform.hpp"
#include "Core/EngineApi.hpp"
#include "KeyCode.hpp"

struct GLFWwindow;

namespace Sphynx {
	class SE_API Input {
	public:
		static void Init(GLFWwindow* window);

		static void Update();

		static const bool IsKeyUp(KeyCode key);
		static const bool IsKeyDown(KeyCode key);
		static const bool IsKeyPressed(KeyCode key);
		static const bool IsKeyReleased(KeyCode key);
		static const ButtonState GetKey(KeyCode key);

		static const bool IsMouseButtonUp(MouseButton button);
		static const bool IsMouseButtonDown(MouseButton button);
		static const bool IsMouseButtonPressed(MouseButton button);
		static const bool IsMouseButtonReleased(MouseButton button);

		static void SetMousePos(const glm::vec2& newPos);
		static const glm::vec2& GetMousePos();
		static const glm::vec2& GetMouseDelta();

		static void SetCursorMode(CursorMode mode);

	private:
		inline static GLFWwindow* s_Window = nullptr;

		static constexpr size_t KEY_COUNT = 348;
		static constexpr size_t MIN_KEY = 32;
		static constexpr size_t MOUSEBUTTON_COUNT = 6;

		inline static glm::vec2 s_LastMousePos = { 0,0 };
		inline static glm::vec2 s_LastMousePosDelta = { 0,0 };

		inline static std::array<ButtonState, KEY_COUNT> s_KeyStates;
		inline static std::array<bool, KEY_COUNT> s_KeyStateChanged;

		inline static std::array<bool, MOUSEBUTTON_COUNT> s_MouseButtonStates;
		inline static std::array<bool, MOUSEBUTTON_COUNT> s_MouseButtonStateChanged;
	};

	class SE_API ConsoleInput {
	public:
		static void Init();

		static void Shutdown();

		using InputCallback = std::function<void(const std::string&)>;

		static void SetInputCallback(const InputCallback& callback) {
			s_InputCallback = callback;
		}

	private:
		inline static std::thread s_InputThread;
		inline static InputCallback s_InputCallback;
	};
}