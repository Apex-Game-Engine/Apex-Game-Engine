#pragma once
#include "Core/Asserts.h"

namespace apex {

	class AxStreamReader
	{
	public:
		AxStreamReader(char* buffer, size_t size) : m_buffer(buffer), m_readptr(buffer), m_size(size) {}

		template <typename T> 
		T* ReadObject()
		{
			axAssert(m_readptr + sizeof(T) <= m_buffer + m_size);
			T* obj = reinterpret_cast<T*>(m_readptr);
			m_readptr += sizeof(T);
			return obj;
		}

		template <typename T>
		T* ReadArray(size_t element_count)
		{
			axAssert(m_readptr + sizeof(T) * element_count <= m_buffer + m_size);
			T* arr = reinterpret_cast<T*>(m_readptr);
			m_readptr += sizeof(T) * element_count;
			return arr;
		}

	private:
		char* m_buffer;
		char* m_readptr;
		size_t m_size;
	};

	class AxStreamWriter
	{
	public:
		AxStreamWriter(char* buffer, size_t size) : m_buffer(buffer), m_writeptr(buffer), m_size(size) {}

		template <typename T>
		void WriteObject(const T* object)
		{
			axAssert(m_writeptr + sizeof(T) <= m_buffer + m_size);
			memcpy(m_writeptr, object, sizeof(T));
			m_writeptr += sizeof(T);
		}

		template <typename T>
		void WriteArray(const T* arr, size_t element_count)
		{
			axAssert(m_writeptr + sizeof(T) * element_count <= m_buffer + m_size);
			memcpy(m_writeptr, arr, sizeof(T) * element_count);
			m_writeptr += sizeof(T) * element_count;
		}

	private:
		char* m_buffer;
		char* m_writeptr;
		size_t m_size;
	};

	class AxStream
	{
	public:
		constexpr AxStream() : m_buffer(nullptr), m_readptr(nullptr), m_writeptr(nullptr), m_size(0) {}

		AxStream(char* buffer, size_t size) 
		{
			SetBuffer(buffer, size);
		}

		void SetBuffer(char* buffer, size_t size)
		{
			m_buffer = buffer;
			m_readptr = buffer;
			m_writeptr = buffer;
			m_size = size;
		}

		// Read operations
		template <typename T>
		T* ReadObject()
		{
			axAssert(m_readptr + sizeof(T) <= m_buffer + m_size);
			T* obj = reinterpret_cast<T*>(m_readptr);
			m_readptr += sizeof(T);
			return obj;
		}

		template <typename T>
		T* ReadArray(size_t element_count)
		{
			axAssert(m_readptr + sizeof(T) * element_count <= m_buffer + m_size);
			T* arr = reinterpret_cast<T*>(m_readptr);
			m_readptr += sizeof(T) * element_count;
			return arr;
		}

		// Write operations
		template <typename T>
		void WriteObject(const T* object)
		{
			axAssert(m_writeptr + sizeof(T) <= m_buffer + m_size);
			memcpy(m_writeptr, object, sizeof(T));
			m_writeptr += sizeof(T);
		}

		template <typename T>
		void WriteArray(const T* arr, size_t element_count)
		{
			axAssert(m_writeptr + sizeof(T) * element_count <= m_buffer + m_size);
			memcpy(m_writeptr, arr, sizeof(T) * element_count);
			m_writeptr += sizeof(T) * element_count;
		}

	private:
		char* m_buffer;
		char* m_readptr;
		char* m_writeptr;
		size_t m_size;
	};

}
