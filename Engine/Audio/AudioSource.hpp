#pragma once

#include "Core/CoreInclude.hpp"
#include "AudioBuffer.hpp"

namespace Sphynx::Audio {
	class SE_API AudioSource {
	public:
		AudioSource();
		~AudioSource();

		void SetBuffer(const AudioBuffer& buffer);
		const AudioBuffer* GetBuffer() const { return m_Buffer; }

		void Start();
		void Resume();
		void Pause();
		void SkipToEnd();

		bool IsPlaying();
		bool IsPaused();
		bool IsFinished();

		void SetPitch(float pitch);
		float GetPitch();
		void SetGain(float gain);
		float GetGain();
		void SetPosition(const glm::vec3& position);
		glm::vec3 GetPosition();
		void SetLooping(bool looping);
		bool IsLooping();
		void SetMinDistance(float distance);
		float GetMinDistance();
		void SetMaxDistance(float distance);
		float GetMaxDistance();

	private:
		AudioSource(const AudioSource&) = delete;
		AudioSource& operator=(const AudioSource&) = delete;

	private:
		unsigned int m_Source = 0;

		const AudioBuffer* m_Buffer = nullptr;

		float m_Pitch = 1.f;
		float m_Gain = 1.f;
		glm::vec3 m_Position = { 0, 0, 0 };
		bool m_Looping = false;
		float m_MinDistance = 6.f;
		float m_MaxDistance = 30.f;
	};
}