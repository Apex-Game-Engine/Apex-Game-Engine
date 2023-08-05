#include "Memory/AxManagedClass.h"
#include "Core/Logging.h"

#include <cstdlib>

#include "Core/Asserts.h"
#include "Memory/AxHandle.h"
#include "Memory/MemoryManager.h"

namespace apex {

	void* AxManagedClass::operator new(size_t size)
	{
		axAssertMsg(false,
			"default new operator called on a AxManagedClass instance!"
			"\nThis might lead to exceptional conditions.");
		return malloc(size);
	}

	void* AxManagedClass::operator new [](size_t size)
	{
		axAssertMsg(false,
			"default new[] operator called on a AxManagedClass instance!"
			"\nThis might lead to exceptional conditions.");
		return malloc(size);
	}

	void AxManagedClass::operator delete(void* ptr)
	{
		if (memory::MemoryManager::checkManaged(ptr))
		{
			memory::MemoryManager::free(ptr);
		}
		else
		{
			free(ptr);
		}
	}

	void AxManagedClass::operator delete [](void* ptr)
	{
		if (memory::MemoryManager::checkManaged(ptr))
		{
			if (memory::MemoryManager::canFree(ptr))
				memory::MemoryManager::free(ptr);
		}
		else
		{
			free(ptr);
		}
	}

	void* AxManagedClass::operator new(size_t size, void* mem)
	{
		return mem;
	}

	void* AxManagedClass::operator new [](size_t size, void* mem)
	{
		return mem;
	}

	void AxManagedClass::operator delete(void*, void*)
	{
		return;
	}

	void AxManagedClass::operator delete [](void*, void*)
	{
		return;
	}

	void* AxManagedClass::operator new(size_t size, AxHandle& handle)
	{
		axAssert(handle.isValid() && handle.getBlockSize() >= size);
		return handle.getAs<void>();
	}

	void* AxManagedClass::operator new [](size_t size, AxHandle& handle)
	{
		axAssert(handle.isValid() && handle.getBlockSize() >= size);
		return handle.getAs<void>();
	}

	void AxManagedClass::operator delete(void* ptr, AxHandle& handle)
	{
		handle.release();
	}

	void AxManagedClass::operator delete [](void* ptr, AxHandle& handle)
	{
		handle.release();
	}

}

#ifndef APEX_ENABLE_TESTS

void* operator new(size_t size)
{
	axLog("new ()");
	return malloc(size);
}

void* operator new [](size_t size)
{
	axLog("new[] ()");
	return malloc(size);
}

void operator delete(void* mem)
{
	axLog("delete ()");
	free(mem);
}

void operator delete [](void* mem)
{
	axLog("delete[] ()");
	free(mem);
}

#endif
