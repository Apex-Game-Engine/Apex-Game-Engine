#include "Core/Files.h"


#ifdef APEX_PLATFORM_WIN32
#	include <windows.h>
#endif

namespace apex {

	enum FileCharMode : u32
	{
		eBinary			= 0x00000010,
		eChar			= 0x00000020,
		eWideChar		= 0x00000040,
		eUtf8			= 0x00000080,

		eCharModeMask	= 0x000000F0,
	};

	enum FileCreateMode : u32
	{
		eCreateNew		= 0x00000100,
		eOpenExisting	= 0x00000200,

		eCreateModeMask	= 0x00000F00,
	};

	namespace {
		FileHandle OpenFile(const char* filename, u32& flags)
		{
		#if APEX_PLATFORM_WIN32
			const DWORD dwDesiredAccess = (flags & FileAccessMode::eRead ? GENERIC_READ : 0) | (flags & FileAccessMode::eWrite ? GENERIC_WRITE : 0);
			const DWORD dwShareMode = flags & FileAccessMode::eRead ? FILE_SHARE_READ : 0;
			const DWORD dwCreationDisposition = flags & FileCreateMode::eOpenExisting ? OPEN_EXISTING : flags & FileCreateMode::eCreateNew ? CREATE_NEW : OPEN_ALWAYS;

			HANDLE hFile = CreateFileA(filename, dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);

			if (!axVerifyFmt(INVALID_HANDLE_VALUE != hFile, "Could not open file : {}", filename))
			{
				return nullptr;
			}

			DWORD dwLastError = GetLastError();
			switch (dwLastError)
			{
			case ERROR_FILE_NOT_FOUND:
				flags = (flags & ~FileCreateMode::eCreateModeMask) | FileCreateMode::eCreateNew;
			case ERROR_FILE_EXISTS:
				flags = (flags & ~FileCreateMode::eCreateModeMask) | FileCreateMode::eOpenExisting;
			}

			return hFile;
		#else
			#error Not implemented!
		#endif
		}

		void CloseFile(FileHandle hFile)
		{
		#if APEX_PLATFORM_WIN32
			axVerify(CloseHandle(hFile));
		#else
			#error Not implemented!
		#endif
		}

		size_t GetFileSize(FileHandle hFile)
		{
		#if APEX_PLATFORM_WIN32
			LARGE_INTEGER liFileSize;
			if (!axVerifyFmt(GetFileSizeEx(hFile, &liFileSize), "Could not retrieve file size!"))
			{
				return 0;
			}
			return liFileSize.QuadPart;
		#else
			#error Not implemented!
		#endif
		}
	}

	File::File(FileHandle handle, size_t size, const char* filename, u32 flags)
		: m_handle(handle), m_size(size), m_filename(filename), m_flags(flags)
	{
	}
	
	File::~File()
	{
		if (m_mappedHandle)
			CloseHandle(m_mappedHandle);
		if (m_handle)
			CloseFile(m_handle);
	}
	
	File::File(File&& other) noexcept
	{
		*this = std::move(other);
	}

	File& File::operator=(File&& other) noexcept
	{
		m_handle = other.m_handle;
		m_size = other.m_size;
		m_filename = other.m_filename;
		other.m_handle = nullptr;
		other.m_size = 0;
		other.m_filename = nullptr;
		return *this;
	}

	File File::CreateOrOpen(char const* filename, FileAccessMode mode)
	{
		axAssertFmt((mode & FileAccessMode::eReadWrite) != 0, "File must have atleast one of Read or Write access modes!");
		u32 flags = mode;
		const FileHandle handle = apex::OpenFile(filename, flags);
		const size_t size = apex::GetFileSize(handle);
		return { handle, size, filename, flags };
	}

	File File::CreateNew(char const* filename, FileAccessMode mode)
	{
		axAssertFmt((mode & FileAccessMode::eReadWrite) != 0, "File must have atleast one of Read or Write access modes!");
		u32 flags = mode | FileCreateMode::eCreateNew;
		const FileHandle handle = apex::OpenFile(filename, flags);
		const size_t size = apex::GetFileSize(handle);
		return { handle, size, filename, flags };
	}

	File File::OpenExisting(char const* filename, FileAccessMode mode)
	{
		axAssertFmt((mode & FileAccessMode::eReadWrite) != 0, "File must have atleast one of Read or Write access modes!");
		u32 flags = mode | FileCreateMode::eOpenExisting;
		const FileHandle handle = apex::OpenFile(filename, flags);
		const size_t size = apex::GetFileSize(handle);
		return { handle, size, filename, flags };
	}

	size_t File::GetSize() const
	{
		return m_size;
	}

	void* File::MapRange(uint32_t offset, size_t size)
	{
	#if APEX_PLATFORM_WIN32
		HANDLE hFileMapping = m_mappedHandle;
		if (!hFileMapping)
		{
			const DWORD flProtect = m_flags & FileAccessMode::eWrite ? PAGE_READWRITE : PAGE_READONLY;
			hFileMapping = CreateFileMappingA(m_handle, nullptr, flProtect, 0, 0, nullptr);
			if (!axVerifyFmt(INVALID_HANDLE_VALUE != hFileMapping, "Could not create file mapping!"))
			{
				return nullptr;
			}
		}

		const DWORD dwDesiredAccess = m_flags & FileAccessMode::eWrite ? FILE_MAP_READ | FILE_MAP_WRITE : FILE_MAP_READ;
		const DWORD dwFileOffsetHigh = offset >> sizeof(DWORD);
		const DWORD dwFileOffsetLow = offset & sizeof(DWORD);
		void* mappedView = MapViewOfFile(hFileMapping, dwDesiredAccess, dwFileOffsetHigh, dwFileOffsetLow, size);
		if (!axVerifyFmt(nullptr != mappedView, "Could not map file!"))
		{
			return nullptr;
		}

		m_mappedHandle = hFileMapping;
		return mappedView;
	#else
		#error Not implemented!
	#endif
	}

	void File::UnmapRange(void* address)
	{
	#if APEX_PLATFORM_WIN32
		axVerify(UnmapViewOfFile(address));
	#else
		#error Not implemented!
	#endif
	}

	size_t File::Read(void* buffer, size_t buffer_size) const
	{
		axAssert(m_flags & FileAccessMode::eRead);
	#if APEX_PLATFORM_WIN32
		OVERLAPPED ol {};
		DWORD dwBytesRead;
		axVerifyFmt(ReadFile(m_handle, buffer, static_cast<DWORD>(buffer_size), &dwBytesRead, &ol), "Could not read from file : {}", m_filename);
		return dwBytesRead;
	#else
		#error Not implemented!
	#endif
	}

	size_t File::Write(const void* buffer, size_t buffer_size)
	{
		axAssert(m_flags & FileAccessMode::eWrite);
	#if APEX_PLATFORM_WIN32
		DWORD dwBytesWritten;
		axVerifyFmt(WriteFile(m_handle, buffer, static_cast<DWORD>(buffer_size), &dwBytesWritten, nullptr), "Could not write to file : {}", m_filename);
		m_size = GetFileSize(m_handle);
		return dwBytesWritten;
	#else
		#error Not implemented!
	#endif
	}
}
