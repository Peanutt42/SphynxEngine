#pragma once

#include "pch.hpp"

namespace Sphynx::Scripting {
	class SE_API ScriptingEngine {
	public:
		static void Init();
		static void Shutdown();

		// temp
		static void Update();
	};
}