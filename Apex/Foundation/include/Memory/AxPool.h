#pragma once

#include <shared_mutex>

#include "PoolAllocator.h"
#include "Containers/AxArray.h"
#include "Core/Socket.h"

namespace apex {
namespace mem {

	template <typename Mutex>
	class AxPoolBase : public PoolAllocator
	{
		using Base = PoolAllocator;

	public:
		AxPoolBase() = default;
		AxPoolBase(u32 elem_count, u32 elem_size)
		{
			Init(elem_count, elem_size);
		}

		~AxPoolBase()
		{
			Shutdown();
		}

		void Init(u32 elem_count, u32 elem_size)
		{
			const u32 poolSize = elem_count * elem_size;
			PoolAllocator::initialize(apex_new char[poolSize], poolSize, elem_size);
		}

		void Shutdown()
		{
			delete[] static_cast<char*>(PoolAllocator::getBase());
			PoolAllocator::shutdown();
		}

		void* Allocate()
		{
			std::unique_lock lock(m_mutex);
			return allocate(Base::getBlockSize());
		}

		void Free(void* ptr)
		{
			std::scoped_lock lock(m_mutex);
			return free(ptr);
		}

		bool ContainsPointer(void* ptr) const
		{
			std::scoped_lock lock(m_mutex);
			return Base::containsPointer(ptr);
		}

		u32 GetFreeCount() const
		{
			return Base::getFreeBlocks();
		}

		u32 GetTotalCount() const
		{
			return Base::getTotalBlocks();
		}

		u32 GetActiveCount() const
		{
			return GetTotalCount() - GetFreeCount();
		}

		
		void* GetBase() const
		{
			return Base::getBase();
		}

	private:
		mutable Mutex m_mutex;
	};

	template <typename Elem, typename Mutex>
	class AxPool : public AxPoolBase<Mutex>
	{
		using Base = AxPoolBase<Mutex>;

	public:
		AxPool() = default;
		AxPool(u32 elem_count) : Base(elem_count, sizeof(Elem)) {}

		void Init(u32 elem_count) { Base::Init(elem_count, sizeof(Elem)); }

		using Base::Shutdown;

		template <typename... Args>
		Elem* New(Args&&... args)
		{
			return new (Base::Allocate()) Elem(std::forward<Args>(args)...);
		}

		void Delete(Elem* elem)
		{
			elem->~Elem();
			Base::Free(elem);
		}

		Elem* TryGetPtr(u32 index)
		{
			if (IsAlive(index))
			{
				return GetPtrUnsafe(index);
			}
			return nullptr;
		}

		Elem* GetPtr(u32 index)
		{
			Elem* ptr = TryGetPtr(index);
			axAssert(ptr);
			return ptr;
		}

		Elem* GetPtrUnsafe(u32 index)
		{
			return static_cast<Elem*>(Base::GetBase()) + index;
		}

		bool IsAlive(u32 index)
		{
			axAssert(index < Base::GetTotalCount());
			return !Base::isFree(static_cast<Elem*>(Base::GetBase()) + index);
		}

		u32 GetIndex(Elem* ptr)
		{
			axAssert(Base::ContainsPointer(ptr));
			return ptr - Base::GetBase();
		}

		using Base::GetFreeCount;
		using Base::GetTotalCount;
		using Base::GetActiveCount;
	};

}
}

#define DECLARE_POOL(TYPE)															\
	static apex::mem::AxPool<TYPE, std::mutex> s_pool;								\
	static void InitPool(size_t);													\
	static void ShutdownPool();														\
	void* operator new(size_t size, apex::mem::Tag) { return s_pool.Allocate(); }	\
	void* operator new(size_t size, apex::mem::Tag tag, const char* func, const char* file, uint32_t line) { return s_pool.Allocate(); } \
	void* operator new(size_t size, apex::mem::Tag tag, const char* type, const char* func, const char* file, uint32_t line) { return s_pool.Allocate(); } \
	void operator delete(void* ptr) { if (s_pool.ContainsPointer(ptr)) s_pool.Free(ptr); else ::free(ptr); }

#define DEFINE_POOL_MEMBERS(TYPE)										\
	apex::mem::AxPool<TYPE, std::mutex> TYPE::s_pool;					\
	void TYPE::InitPool(size_t max_count) { s_pool.Init(max_count); }	\
	void TYPE::ShutdownPool() { s_pool.Shutdown(); }
