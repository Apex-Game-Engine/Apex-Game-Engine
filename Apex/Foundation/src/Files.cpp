#include "Core/Files.h"


#ifdef APEX_PLATFORM_WIN32
#	include <windows.h>
#endif

namespace apex {

	enum FileCharMode : u32
	{
		eBinary   = 0x00000010,
		eChar     = 0x00000020,
		eWideChar = 0x00000040
	};

	enum FileCreateMode : u32
	{
		eCreateNew    = 0x00000100,
		eOpenExisting = 0x00000200
	};

	static FileHandle OpenFile(const char* filename, u32 mode)
	{
	#ifdef APEX_PLATFORM_WIN32
		const DWORD dwDesiredAccess = (mode & FileAccessMode::eRead ? GENERIC_READ : 0) | (mode & FileAccessMode::eWrite ? GENERIC_WRITE : 0);
		const DWORD dwShareMode = (mode & FileAccessMode::eRead ? FILE_SHARE_READ : 0) | (mode & FileAccessMode::eWrite ? FILE_SHARE_WRITE : 0);
		const DWORD dwCreationDisposition = mode & FileCreateMode::eOpenExisting ? OPEN_EXISTING : mode & FileCreateMode::eCreateNew ? CREATE_NEW : OPEN_ALWAYS;

		HANDLE hFile = CreateFileA(filename, dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);

		if (INVALID_HANDLE_VALUE == hFile)
		{
			axAssertFmt(false, "Could not open file : {}", filename);
			return nullptr;
		}

		return hFile;
	#else
		#error Not implemented!
	#endif
	}

	void CloseFile(FileHandle hFile)
	{
		CloseHandle(hFile);
	}

	static size_t GetFileSize(FileHandle hFile)
	{
	#ifdef APEX_PLATFORM_WIN32
		LARGE_INTEGER liFileSize;
		if (false == GetFileSizeEx(hFile, &liFileSize))
		{
			axAssertFmt(false, "Could not retrieve file size!");
			return 0;
		}
		return liFileSize.QuadPart;
	#endif
	}

	File File::CreateNew(char const* filename, FileAccessMode mode)
	{
		File file;
		file.Create(filename, mode);
		return file;
	}

	File File::OpenExisting(char const* filename, FileAccessMode mode)
	{
		File file;
		file.Open(filename, mode);
		return file;
	}

	File::~File()
	{
		CloseFile(m_handle);
	}

	void File::Create(const char* filename, FileAccessMode mode)
	{
		axAssertFmt((mode & FileAccessMode::eReadWrite) != 0, "File must have atleast one of Read or Write access modes!");
		m_handle = apex::OpenFile(filename, mode | FileCreateMode::eCreateNew);
		m_size = apex::GetFileSize(m_handle);
	#if APEX_CONFIG_DEBUG
		m_filename = filename;
	#endif
	}

	void File::Open(const char* filename, FileAccessMode mode)
	{
		axAssertFmt((mode & FileAccessMode::eReadWrite) != 0, "File must have atleast one of Read or Write access modes!");
		m_handle = apex::OpenFile(filename, mode | FileCreateMode::eOpenExisting);
		m_size = apex::GetFileSize(m_handle);
	}

	AxArray<char> File::Read()
	{
		AxArray<char> buf;
		Read(buf);
		return buf;
	}

	void File::Read(AxArray<char>& buf)
	{
	#ifdef APEX_PLATFORM_WIN32
		buf.resize(m_size);
		OVERLAPPED ol {};
		DWORD dwBytesRead;
		if (FALSE == ::ReadFile(m_handle, buf.data(), static_cast<DWORD>(m_size), &dwBytesRead, &ol))
		{
		#if APEX_CONFIG_DEBUG
			axAssertFmt(false, "Could not read file : {}", m_filename);
		#else
			axAssertFmt(false, "Could not read file!");
		#endif
		}
	#else
		#error Not implemented!
	#endif	
	}

}
