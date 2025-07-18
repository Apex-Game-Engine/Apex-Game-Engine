#pragma once
#include <bitset>

#include "PoolAllocator.h"
#include "Containers/AxArray.h"

namespace apex {
namespace mem {

	class AxBasePool : public PoolAllocator
	{
	public:
		AxBasePool() = default;
		AxBasePool(u32 elem_count, u32 elem_size);
		~AxBasePool();

		void Init(u32 elem_count, u32 elem_size);
		void Shutdown();

		void* Allocate()
		{
			return allocate(PoolAllocator::getBlockSize());
		}

		void Free(void* ptr)
		{
			return free(ptr);
		}

		bool ContainsPointer(void* ptr) const
		{
			return PoolAllocator::containsPointer(ptr);
		}

	private:
		//u32 m_blockSize;
		//u32 m_blockCount;
	};

	template <typename Elem>
	class AxPool : public AxBasePool
	{
	public:
		AxPool() = default;
		AxPool(u32 elem_count) : AxBasePool(elem_count, sizeof(Elem)) {}

		void Init(u32 elem_count) { AxBasePool::Init(elem_count, sizeof(Elem)); }
		using AxBasePool::Shutdown;

		template <typename... Args>
		Elem* New(Args&&... args)
		{
			return new (Allocate()) Elem(std::forward<Args>(args)...);
		}

		void Delete(Elem* elem)
		{
			elem->~Elem();
			AxBasePool::Free(elem);
		}
	};

}
}

#define DECLARE_POOL(TYPE)														   \
	static apex::mem::AxPool<TYPE> s_pool;										   \
	static void InitPool(size_t);												   \
	static void ShutdownPool();													   \
	void* operator new(size_t size, apex::mem::Tag) { return s_pool.Allocate(); } \
	void* operator new(size_t size, apex::mem::Tag tag, const char* func, const char* file, uint32_t line) { return s_pool.Allocate(); } \
	void* operator new(size_t size, apex::mem::Tag tag, const char* type, const char* func, const char* file, uint32_t line) { return s_pool.Allocate(); } \
	void operator delete(void* ptr) { if (s_pool.ContainsPointer(ptr)) s_pool.Free(ptr); else ::free(ptr); }

#define DEFINE_POOL_MEMBERS(TYPE)										\
	apex::mem::AxPool<TYPE> TYPE::s_pool;									\
	void TYPE::InitPool(size_t max_count) { s_pool.Init(max_count); }	\
	void TYPE::ShutdownPool() { s_pool.Shutdown(); }
