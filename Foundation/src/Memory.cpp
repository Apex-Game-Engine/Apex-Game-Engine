#include "Core/Logging.h"

#include <cstdlib>

#include "Core/Asserts.h"
#include "Memory/AxHandle.h"
#include "Memory/MemoryManager.h"


namespace apex::mem {

	void GlobalMemoryOperators::OperatorDelete(void* ptr) noexcept
	{
		if (!MemoryManager::checkManaged(ptr))
		{
		#ifndef APEX_ENABLE_TESTS
			axWarn("Calling ::free on a pointer!");
		#endif
			free(ptr);
			return;
		}
		axVerifyFmt(MemoryManager::canFree(ptr), "Attempting to delete an address within an allocation!");
		MemoryManager::free(ptr);
	}

	void* GlobalMemoryOperators::OperatorNew(size_t size, AxHandle handle)
	{
		if (!handle.isValid())
		{
			handle.allocate(size);
		}
		axAssert(handle.getBlockSize() >= size);
		return handle.getAs<void>();
	}
}


void operator delete(void* ptr) noexcept
{
	apex::mem::GlobalMemoryOperators::OperatorDelete(ptr);
}

void operator delete[](void* ptr) noexcept
{
	apex::mem::GlobalMemoryOperators::OperatorDelete(ptr);
}

void* operator new(size_t size, apex::AxHandle handle)
{
	return apex::mem::GlobalMemoryOperators::OperatorNew(size, handle);
}

void* operator new[](size_t size, apex::AxHandle handle)
{
	return apex::mem::GlobalMemoryOperators::OperatorNew(size, handle);
}

void* operator new(size_t size, apex::AxHandle handle, const char* func, const char* file, uint32_t line)
{
	return apex::mem::GlobalMemoryOperators::OperatorNew(size, handle);
}


#ifndef APEX_ENABLE_TESTS
void* operator new(size_t size)
{
	axError("Using naked new!");
	return malloc(size);
}

void* operator new[](size_t size)
{
	axError("Using naked new[]!");
	return malloc(size);
}
#endif

