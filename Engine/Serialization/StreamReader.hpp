#pragma once

#include "Core/EngineApi.hpp"
#include "std.hpp"

namespace Sphynx {
	class SE_API StreamReader {
	public:
		virtual ~StreamReader() = default;

		virtual bool IsStreamGood() const = 0;
		virtual size_t GetStreamPosition() = 0;
		virtual void SetStreamPosition(size_t position) = 0;
		virtual bool ReadData(char* destination, size_t size) = 0;

		operator bool() const { return IsStreamGood(); }

		template<typename T>
		bool Read(T& type) {
			bool success = ReadData((char*)&type, sizeof(T));
			if (!success)
				throw std::runtime_error("Failed to read data");
			return success;
		}

		template<>
		bool Read<std::string>(std::string& type) {
			size_t size = 0;
			if (!Read(size))
				return false;

			type.resize(size);
			return ReadData((char*)type.data(), size);
		}

		template<>
		bool Read<std::wstring>(std::wstring& type) {
			size_t size = 0;
			if (!Read(size))
				return false;

			type.resize(size);
			return ReadData((char*)type.data(), size * sizeof(wchar_t));
		}

		template<typename Key, typename Value>
		void ReadMap(std::map<Key, Value>& map) {
			size_t size = 0;
			Read<size_t>(size);

			for (size_t i = 0; i < size; i++) {
				Key key;
				Read<Key>(key);
				Read<Value>(map[key]);
			}
		}

		template<typename Key, typename Value>
		void ReadMap(std::unordered_map<Key, Value>& map) {
			size_t size = 0;
			Read<size_t>(size);

			for (size_t i = 0; i < size; i++) {
				Key key;
				Read<Key>(key);
				Read<Value>(map[key]);
			}
		}

		template<typename T>
		void ReadArray(std::vector<T>& array) {
			size_t size = 0;
			Read<size_t>(size);

			array.resize(size);

			for (size_t i = 0; i < size; i++)
				Read<T>(array[i]);
		}
	};
}