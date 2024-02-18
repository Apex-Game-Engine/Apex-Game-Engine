#include "Core/Formatting.h"

namespace apex {

	namespace detail {
		static char g_fmtBuffer[2048];
	}

	buffer_iterator get_buffer()
	{
		return {detail::g_fmtBuffer, std::size(detail::g_fmtBuffer) };
	}
}
