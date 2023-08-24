#pragma once

#include "Api.h"

namespace apex {
	struct Window;

	struct APEX_API Application
	{
		virtual ~Application() = default;

		static Application* Construct(void*, int, uint32 width, uint32 height, const char* name);

		static Application* Instance()
		{
			return s_pInstance;
		}

		virtual void run() = 0;
		virtual void exit() = 0;

		virtual Window* getWindow() = 0;

		static Application *s_pInstance;
	};

}
