#pragma once
#include <string_view>

#include "AxStringRef.h"
#include "Core/Types.h"

namespace apex {

	template <uint16 N>
	class AxStaticString
	{
	public:
		constexpr explicit AxStaticString(AxStringView str) noexcept
		: AxStaticString{str.m_str, std::make_integer_sequence<uint16, N>{}}
		{}

		constexpr explicit AxStaticString(std::string_view str) noexcept
		: AxStaticString{str.data(), std::make_integer_sequence<uint16, N>{}}
		{}

		constexpr const char* data() const noexcept { return m_chars; }

		constexpr uint16 size() const noexcept { return N; }

		constexpr operator std::string_view() const noexcept { return {data(), size()}; }

	private:
		template <uint16... I>
		constexpr AxStaticString(const char* str, std::integer_sequence<uint16, I...>) noexcept
		: m_chars{static_cast<char>(str[I])..., static_cast<char>('\0')}
		{}

		template <uint16... I>
		constexpr AxStaticString(std::string_view str, std::integer_sequence<uint16, I...>) noexcept
		: m_chars{str[I]..., static_cast<char>('\0')}
		{}

		char m_chars[static_cast<size_t>(N) + 1];
	};

	template <>
	class AxStaticString<0> {
	 public:
	  constexpr explicit AxStaticString() = default;

	  constexpr explicit AxStaticString(AxStringView) noexcept {}

	  constexpr explicit AxStaticString(std::string_view) noexcept {}

	  constexpr const char* data() const noexcept { return nullptr; }

	  constexpr uint16 size() const noexcept { return 0; }

	  constexpr operator std::string_view() const noexcept { return {}; }
	};

}
