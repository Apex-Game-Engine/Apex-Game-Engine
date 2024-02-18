#pragma once
#include "Memory/AxManagedClass.h"

namespace apex {

	/**
	 * \brief Non-owning string reference. A simple wrapper class over const char*
	 */
	class AxStringRef : public AxManagedClass
	{
	public:
		AxStringRef() : m_str(nullptr) {}
		AxStringRef(const char* str) : m_str(str) {}

		constexpr explicit operator bool() const { return static_cast<bool>(m_str); }
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
