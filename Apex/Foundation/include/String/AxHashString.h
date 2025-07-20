#pragma once

#include "AxStringView.h"

namespace apex {

	enum class AxHashStringType { HashOnly, HashAndString };

	template <typename Hasher, AxHashStringType>
	class AxBaseHashString;

	template <typename Hasher>
	class AxBaseHashString<Hasher, AxHashStringType::HashOnly>
	{
	public:
		using HashType = decltype(std::declval<Hasher>()(AxStringView{}));

		constexpr AxBaseHashString() = default;
		constexpr AxBaseHashString(AxStringView str) : m_hash(Hasher{}(str)) {}

		[[nodiscard]] constexpr HashType GetHash() const { return m_hash; }

		friend bool operator==(const AxBaseHashString& l, const AxBaseHashString& r)
		{
			return l.GetHash() == r.GetHash();
		}
		
	private:
		HashType m_hash { 0 };
	};

	template <typename Hasher>
	class AxBaseHashString<Hasher, AxHashStringType::HashAndString> : public AxBaseHashString<Hasher, AxHashStringType::HashOnly>
	{
		using Base = AxBaseHashString<Hasher, AxHashStringType::HashOnly>;
	public:
		constexpr AxBaseHashString() = default;
		constexpr AxBaseHashString(AxStringView str) : Base(str), m_str(str) {}

		[[nodiscard]] constexpr AxStringView GetStr() const { return m_str; }
		
	private:
		AxStringView m_str;
	};

	struct FnvHasher64
	{
		static constexpr u64 kValue = 0xcbf29ce484222325;
		static constexpr u64 kPrime = 0x100000001b3;

		[[nodiscard]] static constexpr u64 Hash(const AxStringView& str, const u64 value = kValue) noexcept
		{
			return (str.length() == 0) ? value : Hash(str.substr(1), (value ^ u64((u8)str[0])) * kPrime);
		}

		[[nodiscard]] constexpr u64 operator()(AxStringView str) const noexcept
		{
			return Hash(str);
		}
	};

	using AxHashString	= AxBaseHashString<FnvHasher64, AxHashStringType::HashAndString>;
	using AxHash		= AxBaseHashString<FnvHasher64, AxHashStringType::HashOnly>;

}

constexpr apex::AxHashString operator""_hs(const char* str, size_t len)
{
	return { apex::AxStringView{ str, len } };
}

namespace std {
	template <> struct hash<apex::AxHashString>
	{
		constexpr size_t operator()(const apex::AxHashString& hs) const noexcept
		{
			return hs.GetHash();
		}
	};
}
