#pragma once

#include "std.hpp"
#include "Logging/Logging.hpp"

namespace Sphynx {
	class Time {
	public:
		static void Sleep(uint64_t milliseconds) {
			auto start = std::chrono::high_resolution_clock::now();
			while ((uint64_t)(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count()) < milliseconds) {}
		}

		static void MicroSleep(uint64_t microseconds) {
			auto start = std::chrono::high_resolution_clock::now();
			while ((uint64_t)(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count()) < microseconds) {}
		}
	};

	struct Timer {
		Timer() {
			Reset();
		}

		void Reset() {
			m_Start = std::chrono::high_resolution_clock::now();
		}

		float Elapsed() {
			return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_Start).count() * .001f * .001f * .001f;
		}

		float ElapsedMillis() {
			return Elapsed() * 1000;
		}

		float ElapsedSeconds() {
			return ElapsedMillis() / 1000;
		}

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
	};

	struct ScopedTimer {
		ScopedTimer(const std::string_view name) : m_Name(name) {}
		~ScopedTimer() {
			float time = m_Timer.ElapsedMillis();
			SE_INFO(Logging::General, "[TIMER] {0:3s} - {1:f}ms, {2:f}fps", m_Name, time, 1.f / time * 1000.f);
		}

	private:
		std::string m_Name;
		Timer m_Timer;
	};
}