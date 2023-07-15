#include "pch.hpp"
#include "ScriptingModule.hpp"

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace Sphynx::Scripting {
#ifdef WINDOWS
	struct ModuleData {
		HMODULE Libary = nullptr;
	};
#endif
	
	Module::Module(const std::filesystem::path& filepath) {
		m_Data = new ModuleData();
		
		SE_ASSERT(std::filesystem::exists(filepath), Logging::Scripting, "{} doesn't exist!", filepath.string());

		m_Data->Libary = LoadLibraryW(filepath.native().c_str());
		SE_ASSERT(m_Data->Libary, Logging::Scripting, "Failed to open {}", filepath.string());
	}

	Module::~Module() {
		if (m_Data->Libary) {
			FreeLibrary(m_Data->Libary);
			m_Data->Libary = nullptr;
		}

		delete m_Data;
	}

	void* Module::_GetFuncAddress(const char* name) {
		return GetProcAddress(m_Data->Libary, name);
	}
}