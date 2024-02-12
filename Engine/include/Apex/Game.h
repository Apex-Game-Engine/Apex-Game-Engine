#pragma once
#include "Memory/AxManagedClass.h"
#include "Memory/UniquePtr.h"

namespace apex {
	struct Application;

	class Game : public AxManagedClass
	{
	public:
		Game() = default;
		virtual ~Game() = default;

		virtual void initialize() = 0;
		virtual void update(float deltaTimeMs) = 0;
		virtual void stop() = 0;

		static UniquePtr<Game> Construct(/*CommandLineArguments args*/);
	};

}
