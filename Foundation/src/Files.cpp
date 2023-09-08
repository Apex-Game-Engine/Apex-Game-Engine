#include "Core/Files.h"


#ifdef APEX_PLATFORM_WIN32
#	include <windows.h>
#endif

namespace apex {

	AxArray<char> readFile(char const* filename, FileMode mode)
	{
		AxArray<char> fileBuf;

	#ifdef APEX_PLATFORM_WIN32
		HANDLE hFile;

		DWORD desiredAccess = (mode & FileModeFlags::eRead ? GENERIC_READ : 0) | (mode & FileModeFlags::eWrite ? GENERIC_WRITE : 0);
		DWORD shareMode = (mode & FileModeFlags::eRead ? FILE_SHARE_READ : 0) | (mode & FileModeFlags::eWrite ? FILE_SHARE_WRITE : 0);
		DWORD creationDisposition = mode & FileModeFlags::eOpenExisting ? OPEN_EXISTING : mode & FileModeFlags::eCreateNew ? CREATE_NEW : OPEN_ALWAYS;

		hFile = CreateFile(
			TEXT(filename),
			desiredAccess,
			shareMode,
			NULL,
			creationDisposition,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (INVALID_HANDLE_VALUE == hFile)
		{
			axAssertMsg(false, "Could not open file!");
			return fileBuf;
		}

		LARGE_INTEGER liFileSize;
		if (FALSE == GetFileSizeEx(hFile, &liFileSize))
		{
			axAssertMsg(false, "Could not retrieve file size!");
			CloseHandle(hFile);
			return fileBuf;
		}

		int64 fileSize = liFileSize.QuadPart + (mode & FileModeFlags::eBinary ? 0 : 1);
		fileBuf.resize(fileSize);
		OVERLAPPED ol {};
		DWORD dwBytesRead;
		if (FALSE == ReadFile(hFile, fileBuf.data(), liFileSize.LowPart, &dwBytesRead, &ol))
		{
			axAssertMsg(false, "Could not retrieve file size!");
			CloseHandle(hFile);
			return fileBuf;
		}

		if ((mode & FileModeFlags::eBinary) == 0 && dwBytesRead > 0 && dwBytesRead <= fileSize - 1)
		{
			fileBuf[dwBytesRead] = '\0';
		}

		CloseHandle(hFile);

		return fileBuf;

	#else
		#error Not yet implemented correctly!
		FILE* file;
		(void)fopen_s(&file, filename, "r");

		axAssertMsg(nullptr != file, "Could not open file!");

		(void)_fseeki64(file, 0, SEEK_END);
		auto sizeInBytes = _ftelli64(file);
	#endif
	}

}
