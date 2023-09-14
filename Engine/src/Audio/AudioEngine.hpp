#pragma once

#include "Core/CoreInclude.hpp"

struct ALCdevice;
struct ALCcontext;

namespace Sphynx::Audio {
	class SE_API AudioEngine {
	public:
		AudioEngine();
		~AudioEngine();

		void Update();

	private:
		ALCdevice* m_Device = nullptr;
		ALCcontext* m_Context = nullptr;
	};
}