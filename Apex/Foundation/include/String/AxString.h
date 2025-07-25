#pragma once
#include "AxStringView.h"
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

		template <size_t SIZE>
		AxString(const char str[SIZE]) : AxString(str, SIZE) {}

		AxString(const char* str) : AxString(str, strlen(str)) {}

		AxString(AxStringView sv) : AxString(sv.data(), sv.length()) {}

		AxString(size_t capacity)
		{
			reserve(capacity);
		}

		AxString(const AxString& other)
		{
			SetCString(other.c_str(), other.GetLength());
		}

		AxString& operator=(const AxString& other)
		{
			reset();
			SetCString(other.c_str(), other.GetLength());
			return *this;
		}

		AxString(AxString&& other) noexcept
		{
			::memcpy_s(&m_storage, sizeof(Storage), &other.m_storage, sizeof(Storage));
			::memset(&other.m_storage, 0, sizeof(Storage));
		}

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
				m_storage.sso.m_isSSO = true;
				m_storage.sso.m_length = 0;
				m_storage.sso.m_str[0] = '\0';
			}
			else
			{
				Allocate(bytes);
				m_storage.non_sso.m_isSSO = false;
				m_storage.non_sso.m_length = 0;
				m_storage.non_sso.m_str[0] = '\0';
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
				delete m_storage.non_sso.m_str;
			}
			memset(&m_storage, 0, sizeof(Storage));
		}

		char* data() { return IsSSO() ? m_storage.sso.m_str : m_storage.non_sso.m_str; }
		[[nodiscard]] const char* data() const { return const_cast<AxString*>(this)->data(); }
		[[nodiscard]] const char* c_str() const { return data(); }

		[[nodiscard]] size_t capacity() const { return IsSSO() ? kSsoBufSize : m_storage.non_sso.m_capacity; }
		[[nodiscard]] size_t GetLength() const { return IsSSO() ? m_storage.sso.m_length : m_storage.non_sso.m_length; }
		[[nodiscard]] size_t size() const { return GetLength(); }

		operator AxStringView() const { return AxStringView{ data(), size() }; }

		operator bool() const { return size() > 0; }

	protected:
		void Allocate(size_t capacity)
		{
			capacity += capacity & 1; // round up to even number
			void* mem = mem::MemoryManager::allocate(capacity);
			m_storage.non_sso.m_str = static_cast<char*>(mem);
			m_storage.non_sso.m_capacity = capacity;
		}

		bool IsSSO() const
		{
			return m_storage.sso.m_isSSO;
		}

		void SetCString(const char* str, size_t len)
		{
			if (len < kSsoBufSize)
			{
				m_storage.sso.m_isSSO = true;
				m_storage.sso.m_length = static_cast<u8>(len);
				::memcpy_s(m_storage.sso.m_str, kSsoBufSize-1, str, len);
				m_storage.sso.m_str[len] = '\0';
			}
			else
			{
				Allocate(len + 1);
				m_storage.non_sso.m_isSSO = false;
				m_storage.non_sso.m_length = len;
				::memcpy_s(m_storage.non_sso.m_str, m_storage.non_sso.m_capacity-1, str, len);
				m_storage.non_sso.m_str[len] = '\0';
			}
		}

		void SetLength(size_t len)
		{
			if (IsSSO())
			{
				m_storage.sso.m_length = static_cast<u8>(len);
			}
			else
			{
				m_storage.non_sso.m_length = len;
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
			} non_sso;
			struct SSO
			{
				u8 m_isSSO : 1;             // 0
				u8 m_length  : 7;           // 1
				char m_str[kSsoBufSize];    // 24
			} sso;
		} m_storage {};

		static_assert(sizeof(m_storage) == 24);
	};


}

