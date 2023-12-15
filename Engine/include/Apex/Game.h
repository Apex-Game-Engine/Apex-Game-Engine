#pragma once
#include "Memory/AxManagedClass.h"
#include "Memory/UniquePtr.h"

namespace apex {

	class Game : public AxManagedClass
	{
	public:
		Game() = default;
		virtual ~Game() = default;

		virtual void run() = 0;
		virtual void stop() = 0;

		static UniquePtr<Game> Construct(/*CommandLineArguments args*/);
	};

}
