#pragma once

#include "Api.h"
#include "Game.h"
#include "Memory/UniquePtr.h"

namespace apex {
	struct Window;

	enum class ApplicationState
	{
		eStopped,
		eRunning,
		ePaused
	};

	struct APEX_API Application
	{
		virtual ~Application() = default;

		static Application* Construct(uint32 width, uint32 height, const char* name, UniquePtr<Game>&& pGame);

		static Application* Instance()
		{
			return s_pInstance;
		}

		virtual void initialize() = 0;
		virtual void run() = 0;
		virtual void exit() = 0;
		virtual void shutdown() = 0;

		virtual Window* getWindow() = 0;
		virtual ApplicationState getState() = 0;

		static Application *s_pInstance;
	};

}
