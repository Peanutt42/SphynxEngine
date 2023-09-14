#pragma once

#include "Core/CoreInclude.hpp"

namespace Sphynx::Audio {
	enum class AudioFormat {
		Mono,
		Stereo
	};

	struct AudioBufferInfo {
		std::vector<short> Data;
		size_t SampleRate = 0;
		AudioFormat Format = AudioFormat::Mono;
	};
	
	class SE_API AudioBuffer {
	public:
		AudioBuffer(const AudioBufferInfo& info);
		~AudioBuffer();

		AudioFormat GetFormat() const { return m_Format; }

		unsigned int GetHandle() const { return m_Buffer; }

	private:
		AudioBuffer(const AudioBuffer&) = delete;
		AudioBuffer& operator=(const AudioBuffer&) = delete;

	private:
		unsigned int m_Buffer = 0;

		AudioFormat m_Format;
	};
}