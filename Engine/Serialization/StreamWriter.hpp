#pragma once

#include "std.hpp"
#include "Core/EngineApi.hpp"

namespace Sphynx {
	class SE_API StreamWriter {
	public:
		virtual ~StreamWriter() = default;

		virtual bool IsStreamGood() const = 0;
		virtual size_t GetStreamPosition() = 0;
		virtual void SetStreamPosition(size_t position) = 0;
		virtual void WriteData(const char* data, size_t size) = 0;

		explicit operator bool() const { return IsStreamGood(); }

		template<typename T>
		void Write(const T& type) {
			WriteData((char*)&type, sizeof(type));
		}

		template<>
		void Write<std::string>(const std::string& type) {
			Write(type.size());
			WriteData((const char*)type.data(), type.size());
		}

		template<>
		void Write<std::string_view>(const std::string_view& type) {
			Write(type.size());
			WriteData((const char*)type.data(), type.size());
		}

		template<>
		void Write<std::wstring>(const std::wstring& type) {
			Write(type.size());
			WriteData((const char*)type.data(), type.size() * sizeof(wchar_t));
		}

		template<>
		void Write<std::wstring_view>(const std::wstring_view& type) {
			Write(type.size());
			WriteData((const char*)type.data(), type.size() * sizeof(wchar_t));
		}


		template<typename Key, typename Value>
		void WriteMap(const std::map<Key, Value>& map) {
			Write<size_t>(map.size());

			for (const auto& [key, value] : map) {
				Write<Key>(key);
				Write<Value>(value);
			}
		}

		template<typename Key, typename Value>
		void WriteMap(const std::unordered_map<Key, Value>& map) {
			Write<size_t>(map.size());

			for (const auto& [key, value] : map) {
				Write<Key>(key);
				Write<Value>(value);
			}
		}

		template<typename T>
		void WriteArray(const std::vector<T>& array) {
			Write<size_t>(array.size());

			for (const auto& element : array)
				Write<T>(element);
		}
	};
}