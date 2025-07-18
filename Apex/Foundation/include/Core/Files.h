#pragma once
#include "Containers/AxArray.h"

namespace apex {

	enum FileAccessMode : u8
	{
		eRead		= 0b01,
		eWrite		= 0b10, 
		eReadWrite	= 0b11,
	};

	using FileHandle = void*;

	class File
	{
	private:
		File(FileHandle handle, size_t size, const char* filename, u32 flags);
	public:
		File() = default;
		~File();
		
		File(File const&) = delete;
		File& operator=(File const&) = delete;

		File(File&&) noexcept;
		File& operator=(File&&) noexcept;

		static File CreateOrOpen(char const* filename, FileAccessMode mode = FileAccessMode::eReadWrite);
		static File CreateNew(char const* filename, FileAccessMode mode = FileAccessMode::eReadWrite);
		static File OpenExisting(char const* filename, FileAccessMode mode = FileAccessMode::eRead);

		size_t GetSize() const;

		void* MapRange(uint32_t offset, size_t size);
		void UnmapRange(void* address);

		size_t Read(void* buffer, size_t buffer_size) const;
		size_t Write(const void* buffer, size_t buffer_size);

	private:
		FileHandle		m_handle = nullptr;
		FileHandle		m_mappedHandle = nullptr;
		size_t			m_size = 0;
		const char*		m_filename = nullptr;
		u32				m_flags = 0;
	};

}
