#pragma once

namespace apex {

	class Console
	{
	public:
		Console(const char* title);
		~Console() = default;

		void connect();
	};

}
