#pragma once
#include "Core/Types.h"
#include "Memory/AxHandle.h"

namespace apex {

	class AxString
	{
	public:
		constexpr AxString() = default;

		AxString(const char* str)
		{
			size_t len = strlen(str);
			if (len < kSsoBufSize)
			{
				m_storage.SSO.m_isSSO = true;
				m_storage.SSO.m_size = static_cast<u8>(len);
				memcpy_s(m_storage.SSO.m_str, kSsoBufSize-1, str, len);
				m_storage.SSO.m_str[len] = '\0';
			}
			else
			{
				Allocate(len);
				m_storage.NonSSO.m_isSSO = false;
				m_storage.NonSSO.m_size = len;
				memcpy_s(m_storage.NonSSO.m_str, m_storage.NonSSO.m_capacity-1, str, len);
				m_storage.NonSSO.m_str[len] = '\0';
			}
		}

		AxString(size_t capacity)
		{
			if (capacity < kSsoBufSize)
			{
				m_storage.SSO.m_isSSO = true;
				m_storage.SSO.m_size = 0;
				m_storage.SSO.m_str[0] = '\0';
			}
			else
			{
				Allocate(capacity);
				m_storage.NonSSO.m_isSSO = false;
				m_storage.NonSSO.m_size = 0;
				m_storage.NonSSO.m_str[0] = '\0';
			}
		}

		~AxString()
		{
			if (!m_storage.SSO.m_isSSO)
			{
				delete m_storage.NonSSO.m_str;
			}
		}

		const char* c_str() const { return m_storage.SSO.m_isSSO ? m_storage.SSO.m_str : m_storage.NonSSO.m_str; }

		size_t capacity() const { return m_storage.SSO.m_isSSO ? kSsoBufSize : m_storage.NonSSO.m_capacity; }
		size_t size() const { return m_storage.SSO.m_isSSO ? m_storage.SSO.m_size : m_storage.NonSSO.m_size; }

	private:
		void Allocate(size_t capacity)
		{
			capacity += capacity & 1;
			AxHandle handle (capacity);
			m_storage.NonSSO.m_str = handle.getAs<char>();
			m_storage.NonSSO.m_capacity = handle.getBlockSize();
		}

	private:
		constexpr static size_t kSsoBufSize = 23;
		union Storage
		{
			struct NonSSO
			{
				size_t m_isSSO    : 1;      // 0
				size_t m_capacity : 63;	    // 8
				size_t m_size;              // 16
				char* m_str;                // 24
			} NonSSO;
			struct SSO
			{
				u8 m_isSSO : 1;             // 0
				u8 m_size  : 7;             // 1
				char m_str[kSsoBufSize];    // 24
			} SSO;
		} m_storage {};
	};


}

