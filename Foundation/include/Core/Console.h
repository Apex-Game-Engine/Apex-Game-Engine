#pragma once

namespace apex {

	class Console
	{
	public:
		Console(const char* title);
		~Console();

		void Write(const char* msg);
		void Error(const char* msg);

	private:
		void* m_hConsoleIn;
		void* m_hConsoleOut;
		void* m_hConsoleErr;
	};

}
