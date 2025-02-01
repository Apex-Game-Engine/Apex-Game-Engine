#pragma once
#include "Containers/AxArray.h"

namespace apex {

	struct FileAccessMode
	{
		enum Value : u32
		{
			eRead  = 0x00000001,
			eWrite = 0x00000002,

			eReadWrite = eRead | eWrite,
		};

		u32 value;

		FileAccessMode(u32 v) : value(v) {}

		constexpr operator u32() const { return value; }
	};

	using FileHandle = void*;

	class File
	{
	public:
		static File CreateNew(char const* filename, FileAccessMode mode = FileAccessMode::eReadWrite);
		static File OpenExisting(char const* filename, FileAccessMode mode = FileAccessMode::eRead);

		File() = default;
		~File();

		File(File const&) = delete;
		File& operator=(File const&) = delete;

		File(File&&) = default;
		File& operator=(File&&) = default;

		void Create(const char* filename, FileAccessMode mode = FileAccessMode::eReadWrite);
		void Open(const char* filename, FileAccessMode mode = FileAccessMode::eRead);

		AxArray<char> Read();

	private:
		FileHandle m_handle {};
		size_t m_size;
	#if APEX_CONFIG_DEBUG
		const char* m_filename;
	#endif
	};

}
