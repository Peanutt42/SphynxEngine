#include "pch.hpp"
#include "AudioEngine.hpp"

#include "ALErrors.hpp"
#include "AudioBuffer.hpp"
#include "AudioSource.hpp"
#include "AudioLoading.hpp"

namespace Sphynx::Audio {
    bool s_Initialized = false;
    ALCdevice* s_Device = nullptr;
    ALCcontext* s_Context = nullptr;


#define TEST_AUDIO true

#if TEST_AUDIO
    AudioBuffer* buffer;
    AudioSource* source;
#endif

    bool AudioEngine::Init() {
        SE_PROFILE_FUNCTION();

        if (s_Initialized)
            return true;

        s_Device = alcOpenDevice(nullptr);
        if (!s_Device) {
            SE_ERR(Logging::Audio, "Failed to open the default audio device");
            return false;
        }

        bool successful = alcCall(s_Device, alcCreateContext, s_Context, s_Device, nullptr);
        if (!successful || !s_Context) {
            SE_ERR(Logging::Audio, "Failed to create context for audio device");
            return false;
        }

        ALCboolean madeContextCurrent = false;
        successful = alcCall(s_Device, alcMakeContextCurrent, madeContextCurrent, s_Context);
        if (!successful || !madeContextCurrent) {
            SE_ERR(Logging::Audio, "Couldn't make audio context current");
            return false;
        }

		SE_INFO(Logging::Audio, "{}", alcGetString(s_Device, ALC_ALL_DEVICES_SPECIFIER));

#if TEST_AUDIO
        source = new AudioSource();

        AudioBufferInfo bufferInfo;
        LoadAudioFile("Resources/Audio/iamtheprotectorofthissystem.wav", bufferInfo);
        buffer = new AudioBuffer(bufferInfo);

        source->SetBuffer(*buffer);
        source->Start();
#endif

        s_Initialized = true;

        return true;
    }

	void AudioEngine::Shutdown() {
        SE_PROFILE_FUNCTION();

        if (!s_Initialized)
            return;

#if TEST_AUDIO
        delete source;
        delete buffer;
#endif

        ALCboolean unsetContext = false;
        if (!alcCall(s_Device, alcMakeContextCurrent, unsetContext, nullptr) || !unsetContext)
            SE_ERR(Logging::Audio, "Failed to unset current audio context");

        if (!alcCall(s_Device, alcDestroyContext, s_Context))
            SE_ERR(Logging::Audio, "Failed to destroy audio context");

        ALCboolean closed = false;
        if (!alcCall(s_Device, alcCloseDevice, closed, s_Device) || !closed)
            SE_ERR(Logging::Audio, "Failed to close opened audio device");

        s_Initialized = false;
	}

	void AudioEngine::Update() {
        SE_PROFILE_FUNCTION();

        if (!s_Initialized)
            return;
	}

    bool AudioEngine::IsInitialized() { return s_Initialized; }
}