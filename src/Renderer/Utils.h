#pragma once
//Utils
#include"Math.h"
#include"Buffer.h"

#include<array>
#include<assert.h>
namespace Utils
{
	

	struct CubeVertex
	{
		TinyMath::Vec4f pos_;
		TinyMath::Vec3f normal_;
	};

	const VertexArrayBuffer<CubeVertex>& GetCubeBuffer();
	

	template<typename SHADER>
	void RenderCube(SHADER& shader)
	{
		const auto& vao = GetCubeBuffer();
		Renderer::Submit(vao, shader);
	}

	template<typename T, size_t MAXLEN>
	class CirCleQueue
	{
	public:
		CirCleQueue() = default;


		bool Empty() const
		{
			return m_front == m_rear;
		}

		bool Full() const
		{
			return (m_rear + 1) % MAXLEN == m_front;
		}

		void Clear()
		{
			m_front = m_rear = 0;
		}

		bool Push(const T& ele)
		{
			if (Full())
			{
				assert(false);
				return false;
			}
			m_data[m_rear] = ele;
			m_rear = (m_rear + 1) % MAXLEN;
			return true;
		}


		bool Pop()
		{
			if (Empty())
			{
				assert(false);
				return false;
			};
			m_front = (m_front + 1) % MAXLEN;
		}
		T Front()const
		{
			if (Empty()) assert(false);
			return m_data[m_front];
		}
		size_t Size() const
		{
			return (size_t)((m_rear - m_front + MAXLEN) % MAXLEN);
		}


	private:
		std::array<T, MAXLEN> m_data;
		int m_front = 0;
		int m_rear = 0;
	};





}