#pragma once
#include "Memory/AxManagedClass.h"

namespace apex {

	/**
	 * \brief Non-owning string reference. A simple wrapper class over const char*
	 */
	class AxStringRef : public AxManagedClass
	{
	private:
		constexpr static char s_nullstr[] = "";

	public:
		AxStringRef() : m_str(s_nullstr) {}
		AxStringRef(const char* str) : m_str(str) {}

		constexpr explicit operator bool() const { return m_str != s_nullstr; }
		constexpr explicit operator const char*() const { return m_str; }

		char const* c_str() const { return m_str; }

	private:
		char const* m_str;
	};

	struct AxStringView
	{
		const char* m_str;
		size_t m_size;
	};

}
