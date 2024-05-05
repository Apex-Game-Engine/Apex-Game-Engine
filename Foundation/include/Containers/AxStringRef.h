#pragma once
#include "Memory/AxManagedClass.h"

namespace apex {

	/**
	 * \brief Non-owning string reference. A simple wrapper class over const char*
	 */
	class AxStringRef : public AxManagedClass
	{
	public:
		enum : size_t { npos = static_cast<size_t>(-1) };
		enum Lazy { eNoLazy=0, eLazy=1 }; // eLazy means that the length is calculated every time it's requested

		AxStringRef() : m_str(nullptr) {}
		AxStringRef(const char* str, Lazy is_lazy = eNoLazy) : m_str(str) { if (is_lazy == eNoLazy) m_length = strlen(str); }
		AxStringRef(const char* str, size_t length) : m_str(str), m_length(length) {}

		constexpr explicit operator bool() const { return static_cast<bool>(m_str); }
		constexpr explicit operator const char*() const { return m_str; }

		char const* c_str() const { return m_str; }
		size_t length(bool recalculate = false) { return (m_length == static_cast<size_t>(-1) || recalculate) ? m_length = strlen(m_str) : m_length; } // stores the length for future use
		size_t length() const { return m_length == static_cast<size_t>(-1) ? strlen(m_str) : m_length; } // calculates the length without storing it

	private:
		char const* m_str;
		size_t m_length = static_cast<size_t>(-1);
	};

	struct AxStringView
	{
		const char* m_str;
		size_t m_size;
	};

}
