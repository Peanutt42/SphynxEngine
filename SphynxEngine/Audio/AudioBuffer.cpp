#include "pch.hpp"
#include "AudioBuffer.hpp"

#include "ALErrors.hpp"

namespace Sphynx::Audio {
	ALenum ToALFormat(AudioFormat format) {
		switch (format) {
		default: SE_FATAL(Logging::Audio, "Invalid AudioFormat format: {}!", (int)format);  return AL_NONE;
		case AudioFormat::Mono:	return AL_FORMAT_MONO16;
		case AudioFormat::Stereo:	return AL_FORMAT_STEREO16;
		}
	}


	AudioBuffer::AudioBuffer(const AudioBufferInfo& info)
		: m_Format(info.Format)
	{
		alCall(alGenBuffers, 1, &m_Buffer);
		alCall(alBufferData, m_Buffer, ToALFormat(m_Format), info.Data.data(), (ALsizei)(info.Data.size() * sizeof(short)), (ALsizei)info.SampleRate);
	}

	AudioBuffer::~AudioBuffer() {
		alCall(alDeleteBuffers, 1, &m_Buffer);
	}
}