#pragma once

#include "Rendering/Texture.hpp"

namespace Sphynx::Editor::Guizmos {
	inline static Rendering::Texture* s_LightBulb = nullptr;
	inline static Rendering::Texture* s_Camera = nullptr;

	inline static void Init() {
		s_LightBulb = new Rendering::Texture("Content/Textures/Guizmos/light_bulb.png");
		s_Camera = new Rendering::Texture("Content/Textures/Guizmos/camera.png");
	}

	inline static void Shutdown() {
		delete s_LightBulb;
		delete s_Camera;
	}
}