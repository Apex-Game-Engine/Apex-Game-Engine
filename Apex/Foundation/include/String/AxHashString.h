#pragma once

#include <ranges>

#include "Containers/AxHashMap.h"
#include "Core/Asserts.h"
#include "Core/Types.h"
#include "Memory/MemoryManager.h"
#include "String/AxConstString.h"
#include "String/AxStringView.h"

namespace apex {

	class StringTable
	{
	public:
		static void Init()
		{
			static char s_instanceBuffer[sizeof(StringTable)];
			sm_instance = new(s_instanceBuffer) StringTable;
		}

		static void Shutdown()
		{
			std::destroy_at(sm_instance);
		}

		static const char* TryAddString(u64 hash, AxStringView str)
		{
			axCriticalAssert(sm_instance);
			auto [it, success] = sm_instance->m_strings.emplace(hash, nullptr);
			if (!success)
			{
				axAssertFmt(str == it->second.c_str(), "AxHashString collision: hash={0:#x}, new='{1}', old='{2}'", hash, str, it->second.c_str());
			}
			else
			{
				it->second.CopyString(str);
			}
			return it->second.c_str();
		}

		static const char* TryGetString(u64 hash)
		{
			axCriticalAssert(sm_instance);
			if (auto it = sm_instance->m_strings.find(hash); it != sm_instance->m_strings.end())
		    {
		        return it->second.c_str();
		    }
		    return nullptr;
		}

	private:
		static StringTable* sm_instance;
		AxDenseHashMap<u64, AxConstString> m_strings;
	};

	enum class AxHashStringType { HashOnly, HashAndString };

	template <typename Hasher, AxHashStringType>
	class AxBaseHashString;

	template <typename Hasher>
	class AxBaseHashString<Hasher, AxHashStringType::HashOnly>
	{
	public:
		//using HashType = decltype(std::declval<Hasher>()(AxStringView{}));
		using HashType = typename Hasher::HashType;

		constexpr AxBaseHashString() = default;
		constexpr AxBaseHashString(HashType hash) : m_hash(hash) {}
		constexpr AxBaseHashString(AxStringView str) : m_hash(Hasher{}(str)) {}

		[[nodiscard]] constexpr HashType GetHash() const { return m_hash; }

		constexpr friend bool operator==(const AxBaseHashString& l, const AxBaseHashString& r)
		{
			return l.GetHash() == r.GetHash();
		}
		
	private:
		HashType m_hash { 0 };
	};

	
	enum eDontSaveString { DontSaveString };

	template <typename Hasher>
	class AxBaseHashString<Hasher, AxHashStringType::HashAndString> : public AxBaseHashString<Hasher, AxHashStringType::HashOnly>
	{
		using Base = AxBaseHashString<Hasher, AxHashStringType::HashOnly>;
	public:
		using typename Base::HashType;

		constexpr AxBaseHashString() = default;
		constexpr AxBaseHashString(HashType hash) : Base(hash) {}
		constexpr AxBaseHashString(AxStringView str) : Base(str)
		{
			StringTable::TryAddString(Base::GetHash(), str);
		}
		constexpr AxBaseHashString(AxStringView str, eDontSaveString) : Base(str) {}

		[[nodiscard]] const char* GetStr() const
		{
			return StringTable::TryGetString(Base::GetHash());
		}
	};

	template <typename _HashType, _HashType kValue, _HashType kPrime>
	struct FnvHasher
	{
		using HashType = _HashType;

		[[nodiscard]] static constexpr HashType Hash(const AxStringView& str, const HashType value = kValue) noexcept
		{
			return (str.length() == 0) ? value : Hash(str.substr(1), (value ^ HashType((u8)str[0])) * kPrime);
		}

		[[nodiscard]] constexpr HashType operator()(AxStringView str) const noexcept
		{
			return Hash(str);
		}
	};

	using FnvHasher64		= FnvHasher<u64, 0xcbf29ce484222325, 0x100000001b3>;
	using FnvHasher32		= FnvHasher<u32, 0x811c9dc5, 0x01000193>;

	using AxHashString		= AxBaseHashString<FnvHasher32, AxHashStringType::HashAndString>;
	using AxHash			= AxBaseHashString<FnvHasher32, AxHashStringType::HashOnly>;

	using AxHashString64	= AxBaseHashString<FnvHasher64, AxHashStringType::HashAndString>;
	using AxHash64			= AxBaseHashString<FnvHasher64, AxHashStringType::HashOnly>;

}

constexpr apex::AxHashString operator""_hs(const char* str, size_t len)
{
	return { apex::AxStringView{ str, len } };
}

constexpr apex::AxHashString operator""_hv(const char* str, size_t len)
{
	return { apex::AxHash{ apex::AxStringView{ str, len } }.GetHash() };
}

constexpr apex::AxHashString64 operator""_hs64(const char* str, size_t len)
{
	return { apex::AxStringView{ str, len } };
}

constexpr apex::AxHashString64 operator""_hv64(const char* str, size_t len)
{
	return { apex::AxHash64{ apex::AxStringView{ str, len } }.GetHash() };
}

namespace std {
	template <> struct hash<apex::AxHashString>
	{
		constexpr size_t operator()(const apex::AxHashString& hs) const noexcept
		{
			return hs.GetHash();
		}
	};

	template <> struct hash<apex::AxHashString64>
	{
		constexpr size_t operator()(const apex::AxHashString64& hs) const noexcept
		{
			return hs.GetHash();
		}
	};
}
