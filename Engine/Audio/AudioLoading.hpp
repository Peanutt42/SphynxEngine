#pragma once

#include "Core/CoreInclude.hpp"
#include "AudioBuffer.hpp"

namespace Sphynx::Audio {
	bool LoadAudioFile(const std::filesystem::path& filepath, AudioBufferInfo& outInfo);
}