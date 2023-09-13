#include "pch.hpp"
#include "AudioEngine.hpp"

#include "ALErrors.hpp"
#include "AudioBuffer.hpp"
#include "AudioSource.hpp"
#include "AudioLoading.hpp"

namespace Sphynx::Audio {
#define TEST_AUDIO true

#if TEST_AUDIO
    AudioBuffer* buffer;
    AudioSource* source;
#endif

    AudioEngine::AudioEngine() {
        SE_PROFILE_FUNCTION();

        m_Device = alcOpenDevice(nullptr);
        SE_ASSERT(m_Device, Logging::Audio, "Failed to open the default audio device");

        bool successful = alcCall(m_Device, alcCreateContext, m_Context, m_Device, nullptr);
        SE_ASSERT(successful && m_Context, Logging::Audio, "Failed to create context for audio device");

        ALCboolean madeContextCurrent = false;
        successful = alcCall(m_Device, alcMakeContextCurrent, madeContextCurrent, m_Context);
        SE_ASSERT(successful && madeContextCurrent, Logging::Audio, "Couldn't make audio context current");

		SE_INFO(Logging::Audio, "Audio output device: {}", alcGetString(m_Device, ALC_ALL_DEVICES_SPECIFIER));

#if TEST_AUDIO
        source = new AudioSource();

        AudioBufferInfo bufferInfo;
        LoadAudioFile("Resources/Audio/iamtheprotectorofthissystem.wav", bufferInfo);
        buffer = new AudioBuffer(bufferInfo);

        source->SetBuffer(*buffer);
        source->Start();
#endif
    }

	AudioEngine::~AudioEngine() {
        SE_PROFILE_FUNCTION();

#if TEST_AUDIO
        delete source;
        delete buffer;
#endif

        ALCboolean unsetContext = false;
        if (!alcCall(m_Device, alcMakeContextCurrent, unsetContext, nullptr) || !unsetContext)
            SE_ERR(Logging::Audio, "Failed to unset current audio context");

        if (!alcCall(m_Device, alcDestroyContext, m_Context))
            SE_ERR(Logging::Audio, "Failed to destroy audio context");

        ALCboolean closed = false;
        if (!alcCall(m_Device, alcCloseDevice, closed, m_Device) || !closed)
            SE_ERR(Logging::Audio, "Failed to close opened audio device");
	}

	void AudioEngine::Update() {
        SE_PROFILE_FUNCTION();
	}
}