#include "Core/Logging.h"

#include <cstdlib>
#if APEX_PLATFORM_WIN32 && _MSC_VER
#include <Windows.h>
#endif

#include <TracyC.h>

#include "Tracy.hpp"
#include "Core/Asserts.h"
#include "Memory/AxHandle.h"
#include "Memory/MemoryManager.h"
#include "Memory/MemoryManagerImpl.h"

#define APEX_ENABLE_MEMORY_LOGS 1

#if APEX_ENABLE_MEMORY_LOGS
#define axMemWarn(...)			axWarn(__VA_ARGS__)
#define axMemDebug(...)			axDebug(__VA_ARGS__)
#define axMemWarnFmt(...)		axWarnFmt(__VA_ARGS__)
#define axMemDebugFmt(...)		axDebugFmt(__VA_ARGS__)
#else
#define axMemWarn(...)
#define axMemDebug(...)
#define axMemWarnFmt(...)
#define axMemDebugFmt(...)
#endif

namespace apex::mem {

	void GlobalMemoryOperators::OperatorDelete(void* ptr) noexcept
	{
		if (ptr == nullptr)
			return;

		if (!MemoryManager::checkManaged(ptr))
		{
		#ifndef APEX_ENABLE_TESTS
			axMemWarn("Calling ::free on a pointer!");
		#endif
			free(ptr);
			return;
		}

		if (axVerifyFmt(MemoryManager::canFree(ptr), "Attempting to delete an address within an allocation!"))
		{
		#ifdef APEX_PROFILE
			TracyFreeS(ptr, 12);
		#endif
			MemoryManager::free(ptr);
		}
	}

	void* GlobalMemoryOperators::OperatorNew(size_t size, AxHandle handle, const char* debug_string = nullptr)
	{
		if (!handle.isValid())
		{
			handle.allocate(size);
		}
		axAssert(handle.getBlockSize() >= size);
	#ifdef APEX_PROFILE
		TracyAllocS(handle.getAs<void>(), handle.getBlockSize(), 12);//, debug_string ? debug_string : "<unknown>");
	#endif
		return handle.getAs<void>();
	}
}

void* operator new(size_t size)
{
	void* ptr = malloc(size);
	//TracyAlloc(ptr, size);

	/*static char buf[512];
	fmt::format_to(buf, "new {} : {}\n", ptr, size)[0] = 0;
	OutputDebugStringA(buf);*/

	return ptr;
}

void* operator new[](size_t size)
{
	void* ptr = malloc(size);
	//TracyAlloc(ptr, size);

	/*static char buf[512];
	fmt::format_to(buf, "new {} : {}\n", ptr, size)[0] = 0;
	OutputDebugStringA(buf);*/

	return ptr;
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

void* operator new[](size_t size, apex::AxHandle handle, const char* func, const char* file, uint32_t line)
{
	return apex::mem::GlobalMemoryOperators::OperatorNew(size, handle);
}

void* operator new(size_t size, apex::AxHandle handle, const char* type, const char* func, const char* file, uint32_t line)
{
	return apex::mem::GlobalMemoryOperators::OperatorNew(size, handle, type);
}

void* operator new[](size_t size, apex::AxHandle handle, const char* type, const char* func, const char* file, uint32_t line)
{
	return apex::mem::GlobalMemoryOperators::OperatorNew(size, handle, type);
}


void operator delete(void* ptr) noexcept
{
	/*static char buf[512];
	fmt::format_to(buf, "delete {}\n", ptr)[0] = 0;
	OutputDebugStringA(buf);*/
	apex::mem::GlobalMemoryOperators::OperatorDelete(ptr);
}

void operator delete[](void* ptr) noexcept
{
	/*static char buf[512];
	fmt::format_to(buf, "delete {}\n", ptr)[0] = 0;
	OutputDebugStringA(buf);*/
	apex::mem::GlobalMemoryOperators::OperatorDelete(ptr);
}

void operator delete(void* ptr, apex::AxHandle /*handle*/)
{
	apex::mem::GlobalMemoryOperators::OperatorDelete(ptr);
}

void operator delete[](void* ptr, apex::AxHandle /*handle*/)
{
	apex::mem::GlobalMemoryOperators::OperatorDelete(ptr);
}


