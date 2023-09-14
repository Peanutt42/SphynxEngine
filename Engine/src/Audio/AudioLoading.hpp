#pragma once

#include "Core/CoreInclude.hpp"
#include "AudioBuffer.hpp"

namespace Sphynx::Audio {
	SE_API bool LoadAudioFile(const std::filesystem::path& filepath, AudioBufferInfo& outInfo);
}