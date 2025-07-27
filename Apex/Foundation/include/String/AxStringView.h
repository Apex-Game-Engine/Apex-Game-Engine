#pragma once

namespace apex {

	/**
	 * \brief Non-owning null-terminated string reference. 
	 * A simple wrapper class over const char*. 
	 */
	class AxStringRef
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

	constexpr size_t cstrlen(const char* str)
	{
		return (*str == 0) ? 0 : 1 + cstrlen(str + 1);
	}

	using AxStringView = std::string_view;

	namespace Strings
	{
		class LineIterator
		{
		public:
			using value_type = AxStringView;

			LineIterator(const AxStringView& str, size_t start = 0)
				: str(str), start(start), end(start)
			{
				Advance();
			}

			LineIterator& operator++()
			{
				Advance();
				return *this;
			}

			LineIterator operator++(int)
			{
				LineIterator it = *this;
				Advance();
				return it;
			}

			operator bool() const
			{
				return start < end && start < str.size();
			}

			void Advance()
			{
				start = str.find_first_not_of("\r\n", end);
				end = str.find_first_of("\r\n", start);
			}

			AxStringView operator*() const { return str.substr(start, end - start); }

		private:
			const AxStringView& str;
			size_t start, end;
		};

	}

}
