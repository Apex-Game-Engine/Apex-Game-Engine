#include "Mount/Mount.h"

namespace apex::mnt {

	enum class PathSearchPhase { Mount, File, Ext };

	bool Path::ValidatePath()
	{
		// mount:path/to/file.ext
		PathSearchPhase phase = PathSearchPhase::Mount;
		const AxStringView path { m_fullpath.data(), m_fullpath.size() + 1 };

		size_t i = 0;
		while (i < path.length())
		{
			char c = path[i];
			switch (phase)
			{
			case PathSearchPhase::Mount:
				{
					while (std::isalnum(c) || c == '_')
						c = path[++i];

					m_mountEnd = static_cast<u16>(i);

					for (size_t d = 0; d < kMountDelimiterLen; d++)
					{
						if (c != kMountDelimiter[d]) return false;
						c = path[++i];
					}

					m_dirStart = static_cast<u16>(i);

					phase = PathSearchPhase::File;
				}
				break;
			case PathSearchPhase::File:
				{
					const u16 nameStart = static_cast<u16>(i);

					while (std::isalnum(c) || c == '_')
						c = path[++i];

					if (c == '/')
					{
						m_dirStart = nameStart;
						i++;
						break;
					}
					if (c == '.' || c == '\0')
					{
						m_fileStart = nameStart;
						i++;
						phase = PathSearchPhase::Ext;
						break;
					}
					return false;
				}
			case PathSearchPhase::Ext:
				{
					m_extStart = static_cast<u16>(i);

					while (std::isalnum(c))
						c = path[++i];

					if (c != 0)
						return false;
					return true;
				}
			}
		}

		return true;
	}

}
