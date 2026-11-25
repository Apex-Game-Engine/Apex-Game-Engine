#pragma once

#include "AxStringView.h"
#include "Core/Asserts.h"
#include "Core/Types.h"
#include "Memory/MemoryManager.h"

namespace apex {

	class AxConstString
	{
	public:
		enum eNoCopy { NoCopy };

		static constexpr u64 kOwnedMask = 1ull << 63;

		AxConstString() = default;
		~AxConstString()
		{
			if (IsOwned())
			{
				delete GetCStr();
			}
		}

		AxConstString(nullptr_t) {}

		AxConstString(const char* str)
		{
			CopyString(str);
		}

		AxConstString(const char* str, eNoCopy)
		{
			SetString(str);
		}

		AxConstString(AxStringView sv)
		{
			CopyString(sv.data(), sv.length());
		}

		// NOTE: `sv` must be null terminated otherwise string operations may lead to out-of-bounds access
		AxConstString(AxStringView sv, eNoCopy)
		{
			SetString(sv.data());
		}

		AxConstString(const AxConstString& other)
		{
			*this = other;
		}

		AxConstString& operator=(const AxConstString& other)
		{
			if (this == &other)
			{
				return *this;
			}
			if (m_str != other.m_str)
			{
				if (IsOwned())
				{
					delete GetCStr();
				}
				if (other.IsOwned())
				{
					CopyString(other.m_str);
				}
			}
			return *this;
		}

		AxConstString(AxConstString&& other) noexcept
		{
			m_str = other.m_str;
			other.m_str = nullptr;
		}

		AxConstString& operator=(AxConstString&& other) noexcept
		{
			m_str = other.m_str;
			other.m_str = nullptr;
			return *this;
		}

		void SetString(const char* str)
		{
			axAssert(((u64)str & ~kOwnedMask) == (u64)str);
			m_str = str;
		}

		void CopyString(const char* str, size_t len = 0)
		{
			if (str == nullptr)
			{
				m_str = nullptr;
				return;
			}
			len = len == 0 ? strlen(str) : len;
			char* newstr = apex_new char[len+1];
			strncpy(newstr, str, len);
			newstr[len] = '\0';
			m_str = (const char*)((u64)newstr | kOwnedMask);
		}

		void CopyString(AxStringView sv)
		{
			CopyString(sv.data(), sv.length());
		}

		bool IsOwned() const
		{
			return ((u64)m_str & kOwnedMask) == kOwnedMask;
		}

		const char* GetCStr() const
		{
			return (const char*)((u64)m_str & ~kOwnedMask);
		}

		size_t CalcLength() const
		{
			return strlen(m_str);
		}

		const char* c_str() const { return GetCStr(); }

		explicit operator const char*() const { return GetCStr(); }

	private:
		const char* m_str = nullptr;
	};

}
