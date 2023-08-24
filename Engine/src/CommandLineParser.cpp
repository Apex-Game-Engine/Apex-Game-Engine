#include "apex_pch.h"
#include "CommandLineParser.h"

#include "Containers/AxRange.h"


namespace apex {

	static char s_tempBuf[1024];

	struct StringBuf
	{
		char buf[2048];
		size_t size { 0 };

		const char* copyFrom(const char* value)
		{
			size_t len = strlen(value);
			const char* ret = buf + size;

			strcpy_s(buf + size, 2048 - size, value);
			size += len + 1;

			return ret;
		}
	};
	static StringBuf s_valueBuf;

	void CommandLineParser::parse(int argc, const char** argv)
	{
		for (int i = 0; i < argc; i++)
		{
			std::string_view argument = argv[i];

			auto range = std::invoke([this, &argument]()
			{
				if (argument.starts_with("--")) // long argument name
				{
					return std::find_if(m_options.begin(), m_options.end(),
					                    [arg = argument.substr(2)](CommandLineOption const& opt)
					                    {
						                    return arg.compare(opt.longName) == 0;
					                    });
				}
				else // short argument name
				{
					return std::find_if(m_options.begin(), m_options.end(),
					                    [arg = argument.substr(1)](CommandLineOption const& opt)
					                    {
						                    return arg.compare(opt.shortName) == 0;
					                    });
				}
			});

			axCheckMsg(range != m_options.end(), "Invalid command line option provided!");


			if (range->hasValue)
			{
				i++;
				range->value = s_valueBuf.copyFrom(argv[i]);
				(void)sprintf_s(s_tempBuf, "Found option '%s' with value '%s'", range->longName, range->value);
				range->set = true;
			}
			else
			{
				(void)sprintf_s(s_tempBuf, "Found option '%s'", range->longName);
				range->set = true;
			}


			axLog(s_tempBuf);
		}
	}
}
