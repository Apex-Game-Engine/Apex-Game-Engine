#pragma once
#include "Containers/AxArray.h"

namespace apex {

	struct FileModeFlags
	{
		enum FileAccessMode : uint32
		{
			eRead  = 0x00000001,
			eWrite = 0x00000002,
		};

		enum FileMode : uint32
		{
			eBinary   = 0x00000010,
			eChar     = 0x00000020,
			eWideChar = 0x00000040
		};

		enum FileCreateMode : uint32
		{
			eCreateNew    = 0x00000100,
			eOpenExisting = 0x00000200
		};
	};
	using FileMode = uint32;

	AxArray<char> readFile(char const* filename, FileMode mode = FileModeFlags::eRead | FileModeFlags::eChar);

}
