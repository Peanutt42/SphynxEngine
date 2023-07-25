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

		uint64_t GetStreamPosition() final {
			return m_Stream.tellp();
		}

		void SetStreamPosition(uint64_t position) final {
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

		uint64_t GetStreamPosition() override {
			return m_Stream.tellg();
		}

		void SetStreamPosition(uint64_t position) override {
			m_Stream.seekg(position);
		}

		bool ReadData(char* destination, size_t size) override {
			m_Stream.read(destination, size);
			return true;
		}

	private:
		std::filesystem::path m_Path;
		std::ifstream m_Stream;
	};

}