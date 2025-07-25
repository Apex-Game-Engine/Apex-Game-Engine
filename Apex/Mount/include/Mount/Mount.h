#pragma once

#include "Containers/AxArray.h"
#include "Containers/AxHashMap.h"
#include "Memory/UniquePtr.h"
#include "String/AxHashString.h"
#include "String/AxStream.h"
#include "String/AxString.h"
#include "String/AxStringView.h"

namespace apex
{
	class AxStream;
}

namespace apex {
namespace mnt {

	class Path
	{
	public:
		Path() = default;
		explicit Path(const char* path)
		{
			if (ValidatePath({ path, strlen(path) + 1 }))
			{
				m_fullpath = path;
			}
		}

		explicit Path(AxString path) : m_fullpath(std::move(path))
		{
			if (!ValidatePath({ m_fullpath.data(), m_fullpath.size() + 1 }))
			{
				m_fullpath.reset();
			}
		}

		constexpr static char kMountDelimiter[] = "://";
		constexpr static size_t kMountDelimiterLen = sizeof(kMountDelimiter) - 1;

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
		bool ValidatePath(AxStringView path);

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

	class IFile
	{
	public:
		virtual bool Open() = 0;
	};

	class File
	{
	public:
		File() : m_file(nullptr) {}
		bool Open() { return m_file->Open();}
	protected:
		File(IFile* file) : m_file(file) {}
	private:
		IFile* m_file;
	};

	class IMount
	{
	public:
		virtual ~IMount() = default;
		virtual const char* GetName() const = 0;
		virtual const char* GetPath() const = 0;
		virtual File* FindFile() const = 0;
	};

	class Mount
	{
	public:
		Mount() : m_mnt(nullptr) {}
		const char* GetName() const { return m_mnt->GetName(); }
		const char* GetPath() const { return m_mnt->GetPath(); }
		File* FindFile() const { return m_mnt->FindFile(); }

		operator bool() const { return m_mnt != nullptr; }

	protected:
		Mount(IMount* mnt) : m_mnt(mnt) {}

	private:
		IMount* m_mnt;

		friend class MountManager;
	};

	class MountManager
	{
	public:
		MountManager() = default;

		void MountDirectory(AxHashString mnt, const char* path);
		void MountArchive(AxHashString mnt, const char* path);
		void MountVirtual(AxHashString mnt);

		bool Exists(const Path& path);
		File* Open(const Path& path);
		Mount GetMount(AxHashString mnt) const;

	private:
		AxDenseHashMap<AxHashString, UniquePtr<IMount>> m_mounts;
	};

	class DirectoryMount : public IMount
	{
	private:
		struct Entry
		{
			enum Flags : u16
			{
				eReadOnly   = 0x00000001,
				eDirectory  = 0x00000002,
				eHidden     = 0x00000004,
				eTemporary  = 0x00000008,
				eCompressed = 0x00000010,
				eEncrypted  = 0x00000020,
			};

			AxString name;
			AxArray<Entry> entries;
			union 
			{
				u64 sizeAndFlags;
				struct
				{
					u64 size : 48;
					u16 flags : 16;
				};
			};

			void BuildRecursive(char* searchPath, size_t pathlen, size_t maxlen);
		};

	public:
		DirectoryMount(AxString mnt, AxString path)
			: m_directoryPath(std::move(path)), m_root(apex::make_unique<Entry>(std::move(mnt)))
		{
			RefreshContents();
		}

		void RefreshContents();

		const char* GetName() const override { return m_root->name.c_str(); }
		const char* GetPath() const override { return m_directoryPath.c_str(); }
		File* FindFile() const override;

	private:
		AxString m_directoryPath;

		UniquePtr<Entry> m_root;
	};

	class VirtualMount : public IMount
	{
	public:
		const char* GetName() const override { return m_name.c_str(); }
		const char* GetPath() const override { return ""; }
		File* FindFile() const override;

	private:
		AxString m_name;
	};

	template <typename Archiver>
	class ArchiveMount : public IMount
	{
	public:
	private:
	};

	constexpr size_t FindMountDelimiter(AxStringView str, size_t index = 0)
	{
		return index < str.size() - std::size("://")
				? (str[index] == ':' && str[index+1] == '/' && str[index+2] == '/')
					? index
					: FindMountDelimiter(str, index + 1)
				: AxStringView::npos;
	}
	
	constexpr AxHashString AxPath(AxStringView str)
	{
		size_t del = FindMountDelimiter(str);
		return del != AxStringView::npos ? AxHashString{ str.substr(del + 3) } : AxHashString{};
	}

	static_assert(AxPath("assets://meshes/characters/CatM_Milio_00.axmesh") == AxHashString("meshes/characters/CatM_Milio_00.axmesh"));

}
}
