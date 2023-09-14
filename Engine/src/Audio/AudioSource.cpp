#include "pch.hpp"
#include "AudioSource.hpp"

#include "ALErrors.hpp"
#include <AL/alext.h>

namespace Sphynx::Audio {
	AudioSource::AudioSource() {
		alCall(alGenSources, 1, &m_Source);
		alCall(alSourcei, m_Source, AL_BUFFER, 0);

		SetPitch(m_Pitch);
		SetGain(m_Gain);
		SetPosition(m_Position);
		SetLooping(m_Looping);
		SetMinDistance(m_MinDistance);
		SetMaxDistance(m_MaxDistance);
	}

	AudioSource::~AudioSource() {
		alCall(alSourceStop, m_Source);
		alCall(alDeleteSources, 1, &m_Source);
	}

	void AudioSource::SetBuffer(const AudioBuffer& buffer) {
		m_Buffer = &buffer;
		ALint prevBuffer = 0;
		alGetSourcei(m_Source, AL_BUFFER, &prevBuffer);
		if (m_Buffer->GetHandle() != (ALuint)prevBuffer) {
			alSourcei(m_Source, AL_BUFFER, m_Buffer->GetHandle());
			alSourceRewind(m_Source);
		}
	}

	void AudioSource::Start() {
		glm::vec3 sourcePosition{0, 0, 0};
		alGetSource3f(m_Source, AL_POSITION, &sourcePosition.x, &sourcePosition.y, &sourcePosition.z);
		if (m_Buffer && m_Buffer->GetFormat() != AudioFormat::Mono && sourcePosition != glm::vec3{ 0, 0, 0 })
			SE_WARN(Logging::Audio, "Trying to play non-mono (>1 channel) audio with a 3D position! Change position to zero or convert audio to mono");
		
		alSourcePlay(m_Source);
	}

	void AudioSource::Resume() {
		int state = 0;
		alGetSourcei(m_Source, AL_SOURCE_STATE, &state);
		if (state != AL_PAUSED)
			return;

		alSourcePlay(m_Source);
	}

	void AudioSource::Pause() {
		int state = 0;
		alGetSourcei(m_Source, AL_SOURCE_STATE, &state);
		if (state != AL_PAUSED)
			alSourcePause(m_Source);
	}

	void AudioSource::SkipToEnd() {
		int state = 0;
		alGetSourcei(m_Source, AL_SOURCE_STATE, &state);
		if (state != AL_STOPPED)
			alSourceStop(m_Source);
	}

	bool AudioSource::IsPlaying() {
		int state = 0;
		alGetSourcei(m_Source, AL_SOURCE_STATE, &state);
		return state == AL_PLAYING;
	}

	bool AudioSource::IsPaused() {
		int state = 0;
		alGetSourcei(m_Source, AL_SOURCE_STATE, &state);
		return state == AL_PAUSED;
	}

	bool AudioSource::IsFinished() {
		int state = 0;
		alGetSourcei(m_Source, AL_SOURCE_STATE, &state);
		return state == AL_STOPPED;
	}

	void AudioSource::SetPitch(float pitch) {
		m_Pitch = pitch;
		alSourcef(m_Source, AL_PITCH, m_Pitch);
	}

	float AudioSource::GetPitch() {
		alGetSourcef(m_Source, AL_PITCH, &m_Pitch);
		return m_Pitch;
	}

	void AudioSource::SetGain(float gain) {
		m_Gain = gain;
		alSourcef(m_Source, AL_GAIN, m_Gain);
	}

	float AudioSource::GetGain() {
		alGetSourcef(m_Source, AL_GAIN, &m_Gain);
		return m_Gain;
	}

	void AudioSource::SetPosition(const glm::vec3& position) {
		m_Position = position;
		alSource3f(m_Source, AL_POSITION, m_Position.x, m_Position.y, m_Position.z);
	}

	glm::vec3 AudioSource::GetPosition() {
		alGetSource3f(m_Source, AL_POSITION, &m_Position.x, &m_Position.y, &m_Position.z);
		return m_Position;
	}

	void AudioSource::SetLooping(bool looping) {
		m_Looping = looping;
		alSourcei(m_Source, AL_LOOPING, m_Looping ? AL_TRUE : AL_FALSE);
	}

	bool AudioSource::IsLooping() {
		ALint looping = AL_FALSE;
		alGetSourcei(m_Source, AL_LOOPING, &looping);
		m_Looping = looping == AL_TRUE;
		return m_Looping;
	}

	void AudioSource::SetMinDistance(float distance) {
		m_MinDistance = distance;
		alSourcef(m_Source, AL_REFERENCE_DISTANCE, m_MinDistance);
	}

	float AudioSource::GetMinDistance() {
		alGetSourcef(m_Source, AL_REFERENCE_DISTANCE, &m_MinDistance);
		return m_MinDistance;
	}

	void AudioSource::SetMaxDistance(float distance) {
		m_MaxDistance = distance;
		alSourcef(m_Source, AL_MAX_DISTANCE, m_MaxDistance);
	}

	float AudioSource::GetMaxDistance() {
		alGetSourcef(m_Source, AL_MAX_DISTANCE, &m_MaxDistance);
		return m_MaxDistance;
	}
}