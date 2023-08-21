#pragma once

#include "std.hpp"
#include "Core/EngineApi.hpp"
#include "StreamWriter.hpp"
#include "StreamReader.hpp"

namespace Sphynx {
	class SE_API FileStreamWriter : public StreamWriter {
	public:
		FileStreamWriter(const std::filesystem::path& path)
			: m_Path(path)
		{
			m_Stream = std::ofstream(path, std::ifstream::out | std::ifstream::binary);
		}

		FileStreamWriter(const FileStreamWriter&) = delete;

		virtual ~FileStreamWriter() {
			m_Stream.close();
		}


		bool IsStreamGood() const final {
			return m_Stream.good();
		}

		uint64 GetStreamPosition() final {
			return m_Stream.tellp();
		}

		void SetStreamPosition(uint64 position) final {
			m_Stream.seekp(position);
		}

		void WriteData(const char* data, size_t size) final {
			m_Stream.write(data, size);
		}

	private:
		std::filesystem::path m_Path;
		std::ofstream m_Stream;
	};

	class SE_API FileStreamReader : public StreamReader {
	public:
		FileStreamReader(const std::filesystem::path& path)
			: m_Path(path)
		{
			SE_ASSERT(std::filesystem::exists(m_Path), "Can't find file {}", m_Path.string());
			m_Stream = std::ifstream(path, std::ifstream::in | std::ifstream::binary);
		}

		FileStreamReader(const FileStreamReader&) = delete;

		~FileStreamReader() {
			m_Stream.close();
		}


		bool IsStreamGood() const final {
			return m_Stream.good();
		}

		uint64 GetStreamPosition() override {
			return m_Stream.tellg();
		}

		void SetStreamPosition(uint64 position) override {
			m_Stream.seekg(position);
		}

		bool ReadData(char* destination, size_t size) override {
			m_Stream.read(destination, size);
			return true;
		}

		template<typename CharT = char>
		static void ReadTextFile(const std::filesystem::path& filepath, std::basic_string<CharT>& outText) {
			SE_ASSERT(std::filesystem::exists(filepath), "Can't find file {}", filepath.string());
			std::ifstream stream(filepath, std::ifstream::in);
			SE_ASSERT(stream, "Can't open file {}", filepath.string());
			std::basic_ostringstream<CharT> ss;
			ss << stream.rdbuf();
			outText = ss.str();
		}

		static void ReadBinaryFile(const std::filesystem::path& filepath, std::vector<byte>& outData) {
			SE_ASSERT(std::filesystem::exists(filepath), "Can't find file {}", filepath.string());
			std::ifstream stream(filepath, std::ifstream::in | std::ios::ate);
			SE_ASSERT(stream, "Can't open file {}", filepath.string());
			std::streampos end = stream.tellg();
			stream.seekg(0, std::ios::beg);
			size_t fileSize = end - stream.tellg();
			outData.resize(fileSize);
			stream.read((char*)outData.data(), fileSize);
		}

	private:
		std::filesystem::path m_Path;
		std::ifstream m_Stream;
	};
}