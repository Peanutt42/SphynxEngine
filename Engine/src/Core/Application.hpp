#pragma once

#include "CoreInclude.hpp"

namespace Sphynx {
	class SE_API Application {
	public:
		virtual void OnCreate() = 0;
		virtual void OnDestroy() = 0;

		virtual void Update() = 0;
		virtual void DrawUI() = 0;
	};
}