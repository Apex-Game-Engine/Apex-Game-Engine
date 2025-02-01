#include "Core/Formatting.h"

namespace apex {

	thread_local static char g_fmtBuffer[2048];

	buffer_iterator get_buffer()
	{
		return {g_fmtBuffer, std::size(g_fmtBuffer) };
	}
}
