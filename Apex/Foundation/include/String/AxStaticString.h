#pragma once
#include <string_view>

#include "AxStringView.h"
#include "Core/Types.h"

namespace apex {

	template <u16 N>
	class AxStaticString
	{
	public:
		constexpr explicit AxStaticString(AxStringView str) noexcept
		: AxStaticString{str.data(), std::make_integer_sequence<u16, N>{}}
		{}

		constexpr const char* data() const noexcept { return m_chars; }

		constexpr u16 size() const noexcept { return N; }

		constexpr operator AxStringView() const noexcept { return {data(), size()}; }

	private:
		template <u16... I>
		constexpr AxStaticString(const char* str, std::integer_sequence<u16, I...>) noexcept
		: m_chars{static_cast<char>(str[I])..., static_cast<char>('\0')}
		{}

		template <u16... I>
		constexpr AxStaticString(std::string_view str, std::integer_sequence<u16, I...>) noexcept
		: m_chars{str[I]..., static_cast<char>('\0')}
		{}

		char m_chars[static_cast<size_t>(N) + 1];
	};

	template <>
	class AxStaticString<0> {
	 public:
	  constexpr explicit AxStaticString() = default;

	  constexpr explicit AxStaticString(AxStringView) noexcept {}

	  constexpr const char* data() const noexcept { return nullptr; }

	  constexpr u16 size() const noexcept { return 0; }

	  constexpr operator AxStringView() const noexcept { return {}; }
	};

}
