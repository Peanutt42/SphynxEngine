#include "pch.hpp"
#include "AudioLoading.hpp"

#include <sndfile.hh>

namespace Sphynx::Audio {
	bool LoadAudioFile(const std::filesystem::path& filepath, AudioBufferInfo& outInfo) {
		SndfileHandle sndfile(filepath.native().c_str());
		int error = sndfile.error();
		if (error) {
			SE_ERR(Logging::Audio, "Failed to load audio file {}, error code: {}", filepath.string(), error);
			return false;
		}
		
		outInfo.SampleRate = (size_t)sndfile.samplerate();

		int channels = sndfile.channels();
		if (channels == 1)
			outInfo.Format = AudioFormat::Mono;
		else if (channels == 2)
			outInfo.Format = AudioFormat::Stereo;
		else {
			SE_ERR(Logging::Audio, "Unsupported channel count of {} in {}", channels, filepath.string());
			return false;
		}

		size_t frameSize = (size_t)sndfile.frames() * (size_t)sndfile.channels();
		outInfo.Data.resize(frameSize);
		sndfile.read(outInfo.Data.data(), outInfo.Data.size());

		return true;
	}
}