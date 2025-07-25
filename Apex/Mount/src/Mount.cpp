#include "Mount/Mount.h"
#include "String/AxHashString.h"

#if APEX_PLATFORM_WIN32
#	define WIN32_LEAN_AND_MEAN
#	include <Windows.h>
#endif

namespace apex::mnt {

	bool Path::ValidatePath(AxStringView path)
	{
		enum class PathSearchPhase { Mount, File, Ext };

		// mount:path/to/file.ext
		PathSearchPhase phase = PathSearchPhase::Mount;

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

	void MountManager::MountDirectory(AxHashString mnt, const char* path)
	{
		auto it = m_mounts.find(mnt);
		if (axVerifyFmt(it == m_mounts.end(), "Mount '{}' already exists!", mnt.GetStr()))
		{
			m_mounts.emplace_hint(it, mnt, apex_new DirectoryMount(mnt.GetStr(), path));
		}
	}

	bool MountManager::Exists(const Path& path)
	{
		return false;
	}

	File* MountManager::Open(const Path& path)
	{
		return nullptr;
	}

	Mount MountManager::GetMount(AxHashString mnt) const
	{
		auto it = m_mounts.find(mnt);
		if (it == m_mounts.end()) return {};
		return { it->second.get() };
	}

	// TODO: Use AxString with Arenas for temporary allocations

	void DirectoryMount::Entry::BuildRecursive(char* searchPath, const size_t pathlen, const size_t maxlen)
	{
		entries.reserve(8);

#if APEX_PLATFORM_WIN32
		WIN32_FIND_DATAA findData;

		searchPath[pathlen  ] = '\\'; // replace '\0' with '\\'
		searchPath[pathlen+1] = '*';  // add '*'
		searchPath[pathlen+2] = '\0'; // add '\0'

		HANDLE hFind = FindFirstFileA(searchPath, &findData);

		auto printWinError = [&searchPath]()
		{
			char errMsgBuf[128];
			DWORD dwErr = GetLastError();
			FormatMessageA(
				FORMAT_MESSAGE_IGNORE_INSERTS |
				FORMAT_MESSAGE_FROM_SYSTEM,
				nullptr,
				dwErr,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				errMsgBuf,
				std::size(errMsgBuf), NULL);

			axErrorFmt("Error in Win32 FindFirstFile '{}' ({}) {}", searchPath, dwErr, errMsgBuf);
		};

		if (INVALID_HANDLE_VALUE == hFind)
		{
			printWinError();
			return;
		}

		do
		{
			if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
			{
				continue;
			}

			u16 flags = 0;
			flags |= ~((findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) - 1) & Entry::eHidden;
			flags |= ~((findData.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) - 1) & Entry::eTemporary;
			flags |= ~((findData.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) - 1) & Entry::eCompressed;
			flags |= ~((findData.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) - 1) & Entry::eEncrypted;

			if (entries.size() == entries.capacity())
			{
				entries.reserve(entries.capacity() * 2);
			}

			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				fmt::println("    <DIR>  {}", findData.cFileName);
				flags |= Entry::eDirectory;
				Entry& entry = entries.emplace_back(AxStringView{ findData.cFileName });
				entry.size = 0;
				entry.flags = flags;

				size_t namelen = strlen(findData.cFileName);
				strcpy_s(searchPath + pathlen + 1, maxlen - pathlen - 1, findData.cFileName); // append child dir name to 'path\\to\\dir' + '\\'
				entry.BuildRecursive(searchPath, pathlen + 1 + namelen, maxlen); // recursive call
				searchPath[pathlen+1] = '*';  // add '*'
				searchPath[pathlen+2] = '\0'; // add '\0'
			}
			else
			{
				LARGE_INTEGER fileSize;
				fileSize.LowPart = findData.nFileSizeLow;
				fileSize.HighPart = findData.nFileSizeHigh;
				fmt::println("    <FILE>  {}  {}B", findData.cFileName, fileSize.QuadPart);
				flags |= ~((findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) - 1) & Entry::eReadOnly;

				Entry& entry = entries.emplace_back(AxStringView{ findData.cFileName });
				entry.size = fileSize.QuadPart;
				entry.flags = flags;
			}
		} while (FindNextFileA(hFind, &findData) != 0);

		const DWORD dwErr = GetLastError();
		if (dwErr != ERROR_NO_MORE_FILES)
		{
			printWinError();
		}

		FindClose(hFind);
#endif
	}

	void DirectoryMount::RefreshContents()
	{
		AxArray<char> strbuf(1024);
		strcpy_s(strbuf.dataMutable(), 1024, m_directoryPath.data());
		m_root->BuildRecursive(strbuf.dataMutable(), m_directoryPath.GetLength(), 1024 - 1);
	}

	File* DirectoryMount::FindFile() const
	{
		return nullptr;
	}
}
