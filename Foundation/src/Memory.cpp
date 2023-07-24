#include "Memory/Memory.h"
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
			memory::MemoryManager::free(ptr);
		}
		else
		{
			free(ptr);
		}
	}

	void* AxManagedClass::operator new(size_t size, AxHandle& handle)
	{
		axAssert(handle.isValid() && handle.getBlockSize() >= size);
		return handle.m_cachedPtr;
	}

	void* AxManagedClass::operator new [](size_t size, AxHandle& handle)
	{
		axAssert(handle.isValid());
		return handle.m_cachedPtr;
	}

	void AxManagedClass::operator delete(void* ptr, AxHandle& handle)
	{
		free(ptr);
	}

	void AxManagedClass::operator delete [](void* ptr, AxHandle& handle)
	{
		free(ptr);
	}

}

#ifndef APEX_ENABLE_TESTS

void* operator new(size_t size)
{
	axInfo("new ()");
	return malloc(size);
}

void* operator new [](size_t size)
{
	axInfo("new[] ()");
	return malloc(size);
}

void operator delete(void* mem)
{
	axInfo("delete ()");
	free(mem);
}

void operator delete [](void* mem)
{
	axInfo("delete[] ()");
	free(mem);
}


#endif
