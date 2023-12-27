#pragma once

#include "pch.hpp"
#include "Rendering/Image.hpp"

namespace Sphynx::Editor::Guizmos {
	inline static Rendering::Image* s_LightBulbImage = nullptr;
	inline static Rendering::Image* s_CameraImage = nullptr;

	inline static void Init() {
		s_LightBulbImage = new Rendering::Image((std::filesystem::path)"Content/Guizmos/light_bulb.png");
		s_CameraImage = new Rendering::Image((std::filesystem::path)"Content/Guizmos/camera.png");
	}

	inline static void Shutdown() {
		delete s_LightBulbImage;
		delete s_CameraImage;
	}
}