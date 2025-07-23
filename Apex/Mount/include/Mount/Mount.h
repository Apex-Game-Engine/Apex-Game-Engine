#pragma once

#include "Containers/AxArray.h"
#include "String/AxString.h"
#include "String/AxStringView.h"

namespace apex {
namespace mnt {

	struct File
	{
		void* handle;
	};

	class IMount;

	class Path
	{
	public:
		Path() = default;
		explicit Path(const char* path) : m_fullpath(path) {}
		explicit Path(AxString path) : m_fullpath(std::move(path)) {}

		constexpr static char kMountDelimiter[] = "://";
		constexpr static size_t kMountDelimiterLen = sizeof(kMountDelimiter) - 1;

		bool ValidatePath();

		[[nodiscard]] bool IsValid() const { return !!m_fullpath; }
		[[nodiscard]] bool HasFileName() const { return m_fileStart != m_fullpath.size(); }
		[[nodiscard]] bool HasFileExtension() const { return !!m_extStart; }

		[[nodiscard]] AxStringView GetMount() const { return m_fullpath ? SubPath(0, m_mountEnd) : AxStringView{}; }
		[[nodiscard]] AxStringView GetDirectoryPath() const { return m_fullpath ? SubPath(m_mountEnd+kMountDelimiterLen, m_fileStart-1) : AxStringView{}; }
		[[nodiscard]] AxStringView GetDirectoryName() const { return m_fullpath ? SubPath(m_dirStart, m_fileStart-1) : AxStringView{}; }
		[[nodiscard]] AxStringView GetFilePath() const { return m_fullpath ? SubPath(m_mountEnd+kMountDelimiterLen) : AxStringView{}; }
		[[nodiscard]] AxStringView GetFileName() const { return m_fullpath ? SubPath(m_fileStart) : AxStringView{}; }
		[[nodiscard]] AxStringView GetFileBaseName() const { return m_fullpath ? SubPath(m_fileStart, m_extStart-1) : AxStringView{}; }
		[[nodiscard]] AxStringView GetFileExtension() const { return m_fullpath && m_extStart ? SubPath(m_extStart) : AxStringView{}; }

	protected:
		[[nodiscard]] AxStringView SubPath(size_t start, size_t end = AxStringView::npos) const
		{
			return AxStringView(m_fullpath).substr(start, end - start);
		}

	private:
		AxString	m_fullpath;
		u16			m_mountEnd { 0 };	// mount : 0..mountEnd
		u16			m_dirStart { 0 };	// dir   : dirStart..fileStart-1 (exclude /)
		u16			m_fileStart { 0 };	// file	 : fileStart..extStart-1 (exclude .)
		u16			m_extStart { 0 };	// ext   : extStart..N
	};

	class MountManager
	{
	public:
		MountManager() = default;

		void Mount(const char* path);



	private:
		AxArray<IMount*> m_mounts;
	};

}
}
