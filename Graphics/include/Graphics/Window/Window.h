#pragma once

namespace apex {

	class Window
	{
	public:
		virtual ~Window() = default;
		
		virtual void show(int nCmdShow) = 0;
		virtual void draw() = 0;
		virtual void pollOSEvents() = 0;
		virtual void waitForOSEvent() = 0;

		virtual void* getOsWindowHandle() = 0;
	};

}