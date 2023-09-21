#pragma once

#include "std.hpp"
#include "Core/IntTypes.hpp"

namespace Sphynx {
	struct BufferView {
		const byte* Data;
		size_t Size;

		BufferView()
			: Data(nullptr), Size(0) {}

		BufferView(const byte* data, size_t size)
			: Data(data), Size(size) {}

		BufferView(const void* data, size_t size)
			: Data((const byte*)data), Size(size) {}

		BufferView(const BufferView&) = default;

		BufferView(const BufferView& other, size_t size)
			: Data(other.Data), Size(size) {}

		template<typename T>
		BufferView(const std::vector<T>& data)
			: Data((const byte*)data.data()), Size(data.size() * sizeof(T)) {}

		template<typename T, size_t N>
		BufferView(const std::array<T, N>& data)
			: Data((const byte*)data.data()), Size(data.size() * sizeof(T)) {}

		template<typename T, size_t N>
		BufferView(const T(&data)[N])
			: Data((const byte*)data), Size(N * sizeof(T)) {}

		operator bool() const { return (bool)Data; }

		const byte& operator[](size_t index) const {
			if (index >= Size)
				throw std::out_of_range("Tried to access index " + std::to_string(index) + " on a bufferview of size " + std::to_string(Size));
			return Data[index];
		}

		template<typename T>
		const T* As() const {
			return (const T*)Data;
		}

		// Get the number of elements in this buffer given a elementType T
		template<typename T>
		size_t GetSize() const {
			return Size / sizeof(T);
		}
	};
}