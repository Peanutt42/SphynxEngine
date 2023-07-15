#pragma once

#include "CoreInclude.hpp"

namespace Sphynx {
	class Application {
	public:
		virtual void OnCreate() = 0;
		virtual void OnDestroy() = 0;

		virtual void Update() = 0;
		virtual void DrawUI() = 0;
	};
}