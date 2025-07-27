#pragma once
#include <string_view>
#include <fmt/core.h>
#include <fmt/format.h>

#define FMT_TYPE_FORMATTER(type, obj, ctx) \
	template <> class fmt::formatter<type> { \
	public: \
		constexpr auto parse(format_parse_context& _ctx) { return _ctx.begin(); } \
		template <typename Context> constexpr auto format(type const&, Context&) const; \
	}; \
	template <typename Context> \
	constexpr auto fmt::formatter<type>::format(type const& obj, Context& ctx) const

namespace apex {

	struct buffer_iterator
	{
		char *data;
		size_t size;
	};

	buffer_iterator get_buffer();

	template <typename... Args>
	std::string_view format(fmt::format_string<Args...> fmt, Args&&... args)
	{
		auto [buffer, size] = get_buffer();
		auto res = fmt::format_to_n(buffer, size-1, fmt, std::forward<Args>(args)...);
		buffer[res.size] = '\0'; // null-terminate the string
		return { buffer, res.size };
	}

}
