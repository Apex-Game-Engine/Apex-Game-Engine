#include "Memory/AxPool.h"

#include "Memory/MemoryManager.h"

namespace apex::mem {

	AxBasePool::AxBasePool(u32 elem_count, u32 elem_size)
	{
		Init(elem_count, elem_size);
	}

	void AxBasePool::Init(u32 elem_count, u32 elem_size)
	{
		const u32 poolSize = elem_count * elem_size;
		PoolAllocator::initialize(apex_new char[poolSize], poolSize, elem_size);
	}

	void AxBasePool::Shutdown()
	{
		delete[] static_cast<char*>(PoolAllocator::GetBasePointer());
		PoolAllocator::shutdown();
	}

	AxBasePool::~AxBasePool()
	{
		delete[] static_cast<char*>(PoolAllocator::GetBasePointer());
	}
}
