#pragma once
#include "Core/Asserts.h"
#include "Core/Types.h"
#include "Memory/MemoryManager.h"

namespace apex {

	class AxString
	{
	public:
		constexpr AxString() = default;

		~AxString()
		{
			reset();
		}

		AxString(const char* str, size_t len)
		{
			SetCString(str, len);
		}

		AxString(const char* str)
		{
			const size_t len = strlen(str);
			SetCString(str, len);
		}

		explicit AxString(size_t capacity)
		{
			reserve(capacity);
		}

		AxString(const AxString& other) { *this = other; }

		AxString& operator=(const AxString& other)
		{
			reset();
			const size_t len = other.GetLength();
			SetCString(other.c_str(), other.GetLength());
			return *this;
		}

		AxString(AxString&& other) noexcept { *this = std::move(other); }

		AxString& operator=(AxString&& other) noexcept
		{
			reset();
			::memcpy_s(&m_storage, sizeof(Storage), &other.m_storage, sizeof(Storage));
			::memset(&other.m_storage, 0, sizeof(Storage));
			return *this;
		}

		void reserve(size_t bytes)
		{
			axAssert(capacity() == 0);
			if (bytes < kSsoBufSize)
			{
				m_storage.SSO.m_isSSO = true;
				m_storage.SSO.m_length = 0;
				m_storage.SSO.m_str[0] = '\0';
			}
			else
			{
				Allocate(bytes);
				m_storage.NonSSO.m_isSSO = false;
				m_storage.NonSSO.m_length = 0;
				m_storage.NonSSO.m_str[0] = '\0';
			}
		}

		void resize(size_t length)
		{
			const size_t bytes = length + 1;
			const size_t curCap = capacity();
			const size_t curLen = GetLength();
			if (bytes != curCap && bytes > 0)
			{
				AxString newStr;
				newStr.reserve(bytes);
				::memcpy_s(newStr.data(), newStr.capacity() - 1, data(), std::min(curLen, length));

				*this = std::move(newStr);
			}
			SetLength(length);
			data()[length] = '\0';
		}

		void reset()
		{
			if (!IsSSO())
			{
				delete m_storage.NonSSO.m_str;
			}
			memset(&m_storage, 0, sizeof(Storage));
		}

		char* data() { return IsSSO() ? m_storage.SSO.m_str : m_storage.NonSSO.m_str; }
		[[nodiscard]] const char* data() const { return const_cast<AxString*>(this)->data(); }
		[[nodiscard]] const char* c_str() const { return data(); }

		[[nodiscard]] size_t capacity() const { return IsSSO() ? kSsoBufSize : m_storage.NonSSO.m_capacity; }
		[[nodiscard]] size_t GetLength() const { return IsSSO() ? m_storage.SSO.m_length : m_storage.NonSSO.m_length; }
		[[nodiscard]] size_t size() const { return GetLength(); }

	protected:
		void Allocate(size_t capacity)
		{
			capacity += capacity & 1; // round up to even number
			void* mem = mem::MemoryManager::allocate(capacity);
			m_storage.NonSSO.m_str = static_cast<char*>(mem);
			m_storage.NonSSO.m_capacity = capacity;
		}

		bool IsSSO() const
		{
			return m_storage.SSO.m_isSSO;
		}

		void SetCString(const char* str, size_t len)
		{
			if (len < kSsoBufSize)
			{
				m_storage.SSO.m_isSSO = true;
				m_storage.SSO.m_length = static_cast<u8>(len);
				::memcpy_s(m_storage.SSO.m_str, kSsoBufSize-1, str, len);
				m_storage.SSO.m_str[len] = '\0';
			}
			else
			{
				Allocate(len + 1);
				m_storage.NonSSO.m_isSSO = false;
				m_storage.NonSSO.m_length = len;
				::memcpy_s(m_storage.NonSSO.m_str, m_storage.NonSSO.m_capacity-1, str, len);
				m_storage.NonSSO.m_str[len] = '\0';
			}
		}

		void SetLength(size_t len)
		{
			if (IsSSO())
			{
				m_storage.SSO.m_length = static_cast<u8>(len);
			}
			else
			{
				m_storage.NonSSO.m_length = len;
			}
		}


	private:
		constexpr static size_t kSsoBufSize = 23;
		union Storage
		{
			struct NonSSO
			{
				size_t m_isSSO    : 1;      // 0
				size_t m_capacity : 63;	    // 8
				size_t m_length;            // 16
				char* m_str;                // 24
			} NonSSO;
			struct SSO
			{
				u8 m_isSSO : 1;             // 0
				u8 m_length  : 7;           // 1
				char m_str[kSsoBufSize];    // 24
			} SSO;
		} m_storage {};

		static_assert(sizeof(m_storage) == 24);
	};


}

