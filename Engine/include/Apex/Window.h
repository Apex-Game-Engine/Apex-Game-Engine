#pragma once
#include "Memory/AxManagedClass.h"

namespace apex {

	class Window : public AxManagedClass
	{
	public:
		virtual ~Window() = default;
		
		virtual void show(int nCmdShow) = 0;
		virtual void draw() = 0;
		virtual void close() = 0;
		virtual void pollOSEvents() = 0;
		virtual void waitForOSEvent() = 0;

		virtual void* getOsWindowHandle() = 0;

		virtual void getFramebufferSize(int& width, int& height) = 0;
	};

}
