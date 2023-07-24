#pragma once

namespace apex {

	struct Window
	{
		virtual ~Window() = default;

		virtual void construct(void* hInstance) = 0;
		virtual void show(int nCmdShow) = 0;
		virtual void draw() = 0;
		virtual void pollOSEvents() = 0;
		virtual void waitForOSEvent() = 0;

		virtual void* getOsWindowHandle() = 0;
	};

}