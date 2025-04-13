#pragma once
#include "Api.h"
#include "Containers/AxArray.h"
#include "Containers/AxRange.h"

namespace apex {

	struct CommandLineOption
	{
		const char shortName[2];
		const char longName[30];
		const bool hasValue;
		const char* value;
		bool set = false;
	};

	class APEX_API CommandLineArguments
	{
	public:
		using options_array = AxArray<CommandLineOption>;

		CommandLineArguments() = default;

		CommandLineArguments(options_array&& options)
		: m_options(std::move(options))
		{
		}

		void defineOptions(options_array&& options)
		{
			m_options = std::move(options);
		}
		
		void parse(int argc, char const** argv);
		void parse(const char* cmd_str);

		auto getSelectedOptions()
		{
			return ranges::AxView( m_options, [](const CommandLineOption& opt) { return opt.set; } );
		}

	private:
		options_array m_options;
		
	};

}
