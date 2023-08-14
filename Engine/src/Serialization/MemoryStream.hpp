#pragma once

#include "std.hpp"
#include "Memory/BufferView.hpp"
#include "StreamWriter.hpp"
#include "StreamReader.hpp"

namespace Sphynx {
	class SE_API MemoryStreamWriter : public StreamWriter {
	public:
		MemoryStreamWriter() = default;
		MemoryStreamWriter(const MemoryStreamWriter&) = delete;
		virtual ~MemoryStreamWriter() override = default;

		bool IsStreamGood() const final { return true; }
		size_t GetStreamPosition() override { return m_Position; }
		void SetStreamPosition(size_t position) override { m_Position = position; }
		void WriteData(const char* data, size_t size) final {
			if (m_Position + size >= m_Buffer.size())
				m_Buffer.resize(m_Position + size);
			
			std::memcpy(m_Buffer.data() + m_Position, data, size);
			m_Position += size;
		}

		const std::vector<byte>& GetBuffer() const {
			return m_Buffer;
		}

	private:
		std::vector<byte> m_Buffer;
		size_t m_Position = 0;
	};

	class SE_API MemoryStreamReader : public StreamReader {
	public:
		MemoryStreamReader(BufferView buffer, size_t position = 0)
			: m_Buffer(buffer), m_Position(position) {}
		MemoryStreamReader(const MemoryStreamReader&) = delete;
		virtual ~MemoryStreamReader() override = default;

		bool IsStreamGood() const final { return (bool)m_Buffer; }
		size_t GetStreamPosition() override { return m_Position; }
		void SetStreamPosition(size_t position) override { m_Position = position; }
		bool ReadData(char* destination, size_t size) override {
			bool valid = m_Position + size <= m_Buffer.Size;
			if (!valid)
				return false;

			memcpy(destination, m_Buffer.Data + m_Position, size);
			m_Position += size;
			return true;
		}
		
	private:
		BufferView m_Buffer;
		size_t m_Position = 0;
	};
}