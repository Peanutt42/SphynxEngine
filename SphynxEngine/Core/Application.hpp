#pragma once

#include "CoreInclude.hpp"

namespace Sphynx {
	class Application {
	public:
		Application() = default;

		virtual void OnCreate() = 0;
		virtual void OnDestroy() = 0;

		virtual void Update() = 0;
		virtual void DrawUI() = 0;

		Application(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(const Application&) = delete;
		Application& operator=(Application&&) = delete;
	};
}