#include "Mount/Mount.h"

#include "String/AxHashString.h"

#if APEX_PLATFORM_WIN32
#	define WIN32_LEAN_AND_MEAN
#	include <Windows.h>
#endif

namespace apex::mnt {

	static MountManager* s_mountManager;

	constexpr u64 kMaxPathDepth = 128;

	Path::Path(AxString path)
	{
		std::replace(path.data(), path.data() + path.size(), '\\', '/');
		if (axVerify(ValidatePath({ path.data(), path.size() + 1 })))
		{
			m_fullpath = std::move(path);
		}
	}

	Path Path::ResolveRelative(AxStringView relativePath)
	{
		const char* pathStack[kMaxPathDepth];
		u32 pathDepth = 0;

		auto StackPush = [&](const char* dir)
		{
			axAssert(pathDepth < kMaxPathDepth);
			pathStack[pathDepth++] = dir;
		};

		auto StackPop = [&]()
		{
			axAssert(pathDepth >= 1);
			pathDepth--;
		};

		auto AddPathToResolve = [&](char* path) {
			char *tok, *ctx = nullptr;
			tok = strtok_s(path, "/", &ctx);
			while (tok)
			{
				if (strcmp(tok, ".") == 0)
				{
					continue;
				}
				else if (strcmp(tok, "..") == 0)
				{
					StackPop();
				}
				else
				{
					StackPush(tok);
				}
				tok = strtok_s(nullptr, "/", &ctx);
			}
		};

		AxString basePath { m_fullpath.data() + m_mountEnd + kMountDelimiterLen, (m_fileStart - m_mountEnd - kMountDelimiterLen) };
		AddPathToResolve(basePath.data());
		AxString relPath = relativePath;
		bool isDir = relPath.data()[relPath.GetLength() - 1] == '/';
		AddPathToResolve(relPath.data());

		AxString resolvedPath = m_fullpath.Substr(0, m_mountEnd + kMountDelimiterLen);
		for (u32 i = 0; i < pathDepth; ++i)
		{
			resolvedPath.Append(pathStack[i]);
			if (isDir || i != pathDepth - 1)
			{
				resolvedPath.Append("/");
			}
		}

		return Path{ resolvedPath };
	}

	bool Path::ValidatePath(AxStringView path)
	{
		enum class PathSearchPhase { Mount, File, Ext };

		// mount://path/to/file.ext
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
						if (c != kMountDelimiter[d])
						{
							m_mountEnd = 0;
							i = 0;
							break;
						}
						c = path[++i];
					}
					
					m_dirStart = static_cast<u16>(i);

					phase = PathSearchPhase::File;
				}
				break;
			case PathSearchPhase::File:
				{
					const u16 nameStart = static_cast<u16>(i);

					while (std::isalnum(c) || c == '_' || c == '-')
						c = path[++i];

					if (c == ':')
					{
						char cn = path[i+1];
						if (i > 1 || cn != '/')
						{
							return false;
						}
						i++;
						break;
					}
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

					while (std::isalnum(c) || c == '.')
						c = path[++i];

					if (c != 0)
						return false;
					return true;
				}
			}
		}

		return true;
	}

	void MountManager::Init()
	{
		static char s_mountManagerBuffer[sizeof(MountManager)];
		s_mountManager = new (s_mountManagerBuffer) MountManager;
	}

	void MountManager::Shutdown()
	{
		s_mountManager->~MountManager();
	}

	void MountManager::MountDirectory(AxHashString mnt, const AxString& path)
	{
		MountDirectory(mnt, AxStringView{ path.data(), path.size() });
	}

	void MountManager::MountDirectory(AxHashString mnt, AxStringView path)
	{
		auto it = s_mountManager->m_mounts.find(mnt);
		if (axVerifyFmt(it == s_mountManager->m_mounts.end(), "Mount '{}' already exists!", mnt.GetStr()))
		{
			s_mountManager->m_mounts.emplace_hint(it, mnt, apex_new LocalMount(mnt.GetStr(), path));
		}
	}

	bool MountManager::Exists(const Path& path)
	{
		Mount mount = GetMount(AxHashString(path.GetMount(), DontSaveString));
		if (!mount)
		{
			return false;
		}
		return false;
	}

	File* MountManager::Open(const Path& path)
	{
		return nullptr;
	}

	Mount MountManager::GetMount(AxHashString mnt)
	{
		auto it = s_mountManager->m_mounts.find(mnt);
		if (it == s_mountManager->m_mounts.end()) return {};
		return { it->second.get() };
	}

	bool MountManager::ResolveDevicePath(const Path& path, char *buf, u32 buflen)
	{
		Mount mount = GetMount(AxHashString(path.GetMount(), DontSaveString));
		if (!axVerifyFmt(mount, "Mount point '{}' does not exist!", path.GetMount()))
		{
			return false;
		}
		auto result = fmt::format_to_n(buf, buflen, "{}/{}", mount.GetPath(), path.GetFilePath());
		*result.out = '\0';
		return true;
	}

	// TODO: Use AxString with Arenas for temporary allocations

	void LocalMount::Entry::BuildRecursive(char* searchPath, const size_t pathlen, const size_t maxlen)
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

	void LocalMount::LoadTree()
	{
		AxStaticArray<char, 1024> strbuf;
		strcpy_s(strbuf.data(), 1024, m_directoryPath.data());
		m_root->BuildRecursive(strbuf.data(), m_directoryPath.GetLength(), 1024 - 1);
	}

	File* LocalMount::FindFile() const
	{
		return nullptr;
	}
}
