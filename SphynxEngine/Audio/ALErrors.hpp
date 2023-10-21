#pragma once

#include "Core/CoreInclude.hpp"

#include <AL/al.h>
#include <AL/alc.h>

namespace Sphynx::Audio {
#define alCall(function, ...) alCallImpl(__FILE__, __LINE__, function, __VA_ARGS__)
#define alcCall(device, function, ...) alcCallImpl(__FILE__, __LINE__, device, function, __VA_ARGS__)
#define alCheckErrors() \
	{																				\
		ALenum error = alGetError()													\
		if (error != AL_NO_ERROR) {													\
			std::cerr << "[AL ERROR]: In " << __FILE__ << ":" << __LINE__ << "\n";	\
			std::cerr << alErrorToString(error) << "\n";							\
		}																			\
	}

#define alcCheckErrors(device) \
	{																			\
		ALCenum error = alcGetError(device);									\
		if (error != ALC_NO_ERROR) {											\
			std::cerr << "[ALC ERROR] In " << filename << ":" << line << "\n";	\
			std::cerr << alcErrorToString(error) << "\n";						\
		}																		\
	}

	inline const char* alErrorToString(ALenum error) {
		switch (error) {
		case AL_NO_ERROR:			return "AL_NO_ERROR: No error";
		case AL_INVALID_NAME:		return "AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL function";
		case AL_INVALID_ENUM:		return "AL_INVALID_ENUM: an invalid enum value was passed to an OpenAL function";
		case AL_INVALID_VALUE:		return "AL_INVALID_VALUE: an invalid value was passed to an OpenAL function";
		case AL_INVALID_OPERATION:	return "AL_INVALID_OPERATION: the requested operation is not valid";
		case AL_OUT_OF_MEMORY:		return "AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out of memory";
		default:					return "UNKNOWN AL ERROR";
		}
	}
	inline const char* alcErrorToString(ALCenum error) {
		switch (error) {
		case ALC_NO_ERROR: 			return "ALC_NO_ERROR: No error";
		case ALC_INVALID_VALUE:		return "ALC_INVALID_VALUE: an invalid value was passed to an OpenAL function";
		case ALC_INVALID_DEVICE:	return "ALC_INVALID_DEVICE: a bad device was passed to an OpenAL function";
		case ALC_INVALID_CONTEXT: 	return "ALC_INVALID_CONTEXT: a bad context was passed to an OpenAL function";
		case ALC_INVALID_ENUM: 		return "ALC_INVALID_ENUM: an unknown enum value was passed to an OpenAL function";
		case ALC_OUT_OF_MEMORY: 	return "ALC_OUT_OF_MEMORY: an unknown enum value was passed to an OpenAL function";
		default:					return "UNKOWN ALC ERROR";
		}
	}

	template<typename alFunction, typename... Params>
	auto alCallImpl(const char* filename,
		const std::uint_fast32_t line,
		alFunction function,
		Params... params)
		-> typename std::enable_if_t<!std::is_same_v<void, decltype(function(params...))>, decltype(function(params...))>
	{
		auto ret = function(std::forward<Params>(params)...);
		ALenum error = alGetError();
		if (error != AL_NO_ERROR) {
			std::cerr << "[AL ERROR] In " << filename << ":" << line << "\n";
			std::cerr << alErrorToString(error) << "\n";
			return {};
		}
		return ret;
	}
	template<typename alFunction, typename... Params>
	auto alCallImpl(const char* filename,
		const std::uint_fast32_t line,
		alFunction function,
		Params... params)
		-> typename std::enable_if_t<std::is_same_v<void, decltype(function(params...))>, bool>
	{
		function(std::forward<Params>(params)...);
		ALenum error = alGetError();
		if (error != AL_NO_ERROR) {
			std::cerr << "[AL ERROR] In " << filename << ":" << line << "\n";
			std::cerr << alErrorToString(error) << "\n";
			return false;
		}
		return true;
	}

	template<typename alcFunction, typename... Params>
	auto alcCallImpl(const char* filename,
		const std::uint_fast32_t line,
		ALCdevice* device,
		alcFunction function,
		Params... params)
		-> typename std::enable_if_t<std::is_same_v<void, decltype(function(params...))>, bool>
	{
		function(std::forward<Params>(params)...);
		ALCenum error = alcGetError(device);
		if (error != ALC_NO_ERROR) {
			std::cerr << "[ALC ERROR] In " << filename << ":" << line << "\n";
			std::cerr << alcErrorToString(error) << "\n";
			return false;
		}
		return true;
	}
	template<typename alcFunction, typename ReturnType, typename... Params>
	auto alcCallImpl(const char* filename,
		const std::uint_fast32_t line,
		ALCdevice* device,
		alcFunction function,
		ReturnType& returnValue,
		Params... params)
		-> typename std::enable_if_t<!std::is_same_v<void, decltype(function(params...))>, bool>
	{
		returnValue = function(std::forward<Params>(params)...);
		ALCenum error = alcGetError(device);
		if (error != ALC_NO_ERROR) {
			std::cerr << "[ALC ERROR] In " << filename << ":" << line << "\n";
			std::cerr << alErrorToString(error) << "\n";
			return false;
		}
		return true;
	}

}