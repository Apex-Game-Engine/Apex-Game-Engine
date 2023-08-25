#include "Core/Files.h"


#ifdef APEX_PLATFORM_WIN32
#	include <windows.h>
#endif

namespace apex {

	AxArray<char> readFile(char const* filename)
	{
		AxArray<char> fileBuf;

	#ifdef APEX_PLATFORM_WIN32
		HANDLE hFile;

		hFile = CreateFile(
			TEXT(filename),
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (INVALID_HANDLE_VALUE == hFile)
		{
			axAssertMsg(INVALID_HANDLE_VALUE == hFile, "Could not open file!");
			return fileBuf;
		}

		LARGE_INTEGER liFileSize;
		if (FALSE == GetFileSizeEx(hFile, &liFileSize))
		{
			axAssertMsg(FALSE, "Could not retrieve file size!");
			CloseHandle(hFile);
			return fileBuf;
		}

		int64 fileSize = liFileSize.QuadPart + 1;
		fileBuf.resize(fileSize);
		OVERLAPPED ol {};
		DWORD dwBytesRead;
		if (FALSE == ReadFile(hFile, fileBuf.data(), liFileSize.LowPart, &dwBytesRead, &ol))
		{
			axAssertMsg(FALSE, "Could not retrieve file size!");
			CloseHandle(hFile);
			return fileBuf;
		}

		if (dwBytesRead > 0 && dwBytesRead <= fileSize - 1)
		{
			fileBuf[dwBytesRead] = '\0';
		}

		CloseHandle(hFile);

		return fileBuf;

	#else
		FILE* file;
		(void)fopen_s(&file, filename, "r");

		axAssertMsg(nullptr != file, "Could not open file!");

		(void)_fseeki64(file, 0, SEEK_END);
		auto sizeInBytes = _ftelli64(file);
	#endif
	}

}
