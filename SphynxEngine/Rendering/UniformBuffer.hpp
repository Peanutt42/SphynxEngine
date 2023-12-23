#pragma once

#include "pch.hpp"


namespace Sphynx::Rendering {
	class SE_API UniformBuffer {
	public:
		UniformBuffer(size_t size);

		template<typename T>
		UniformBuffer(int binding) : UniformBuffer(sizeof(T), binding) {}
		
		~UniformBuffer();
		
		void Update(BufferView data);
		
		template<typename T>
		void Update(const T& data) {
			Update(BufferView(&data, sizeof(data)));
		}

		int GetBinding() const { return m_Binding; }

	private:
		uint32 m_ID = 0;
		int m_Size = 0;
		int m_Binding = 0;
		inline static int s_NextBinding = 0;
	};
}