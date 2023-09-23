#pragma once

#include "std.hpp"
#include "IntTypes.hpp"
#include "Logging/Logging.hpp"

namespace Sphynx {
	class SE_API Time {
	public:
		static void Sleep(uint64 milliseconds) {
			auto start = std::chrono::high_resolution_clock::now();
			while (std::chrono::duration_cast<std::chrono::duration<uint64, std::milli>>(std::chrono::high_resolution_clock::now() - start).count() < milliseconds) {}
		}

		static void MicroSleep(uint64 microseconds) {
			auto start = std::chrono::high_resolution_clock::now();
			while (std::chrono::duration_cast<std::chrono::duration<uint64, std::micro>>(std::chrono::high_resolution_clock::now() - start).count() < microseconds) {}
		}
	};

	struct SE_API Timer {
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

	struct SE_API ScopedTimer {
		ScopedTimer(const std::string_view name) : m_Name(name) {}
		~ScopedTimer() {
			[[maybe_unused]] float time = m_Timer.ElapsedMillis();
			SE_INFO("[TIMER] {} - {}ms, {}fps", m_Name, time, 1.f / time * 1000.f);
		}

	private:
		std::string m_Name;
		Timer m_Timer;
	};
}