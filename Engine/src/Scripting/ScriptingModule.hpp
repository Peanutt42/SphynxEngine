#pragma once

#include "Core/CoreInclude.hpp"

namespace Sphynx::Scripting {
	// Platformdependent data
	struct ModuleData;

	class Module {
	public:
		Module(const std::filesystem::path& filepath);
		~Module();

		template<typename Func>
		auto LoadFunction(const std::string_view name) {
			Func function = (Func)_GetFuncAddress(name.data());
			SE_ASSERT(function, Logging::Scripting, "Failed to get function '{}'", name);
			return function;
		}

	private:
		void* _GetFuncAddress(const char* name);

	private:
		ModuleData* m_Data = nullptr;
	};
}