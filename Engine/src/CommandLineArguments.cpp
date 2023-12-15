#include "apex_pch.h"
#include "CommandLineArguments.h"

#include "Containers/AxRange.h"


namespace apex {

	static char s_tempBuf[1024];

	struct StringBuf
	{
		static constexpr size_t MAX_SIZE = 1024;

		char buf[MAX_SIZE];
		size_t size { 0 };

		const char* copyFrom(const char* value)
		{
			size_t len = strlen(value);
			const char* ret = buf + size;

			strcpy_s(buf + size, MAX_SIZE - size, value);
			size += len + 1;

			return ret;
		}

		const char* copyFrom(std::string_view value)
		{
			const char* ret = buf + size;

			apex::memcpy_s<char>(buf + size, MAX_SIZE - size, value.data(), value.size());
			size += value.size() + 1;

			return ret;
		}
	};
	static StringBuf s_valueBuf;

	void CommandLineArguments::parse(int argc, char const** argv)
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
				range->value = argv[++i];
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

	void CommandLineArguments::parse(const char* cmd_str)
	{
		std::string_view cmdStr = cmd_str;
		size_t len = cmdStr.size();

		if (len == 0) return;

		const char* argv[64]; // TODO: consider increasing this size
		int argc = 0;

		size_t offset = cmdStr.find(' ', 0);
		while (offset < cmdStr.size())
		{
			argv[argc++] = s_valueBuf.copyFrom(cmdStr.substr(0, offset));
			cmdStr = cmdStr.substr(offset + 1);
			offset = cmdStr.find(' ', 0);
		}

		argv[argc++] = s_valueBuf.copyFrom(cmdStr);

		parse(argc, argv);
	}
}
