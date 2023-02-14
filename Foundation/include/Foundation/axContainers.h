#pragma once
#include "axAsserts.h"
#include "axMemory.h"


namespace apex {
namespace containers
{

	template <typename Stored_t>
	struct FreeList
	{
		static_assert(sizeof(Stored_t) >= sizeof(Stored_t*));

		union Pointer_t
		{
			Stored_t value;
			Stored_t* pointer;
		};

		FreeList(void* memory, size_t capacity)
			: m_beg(static_cast<Pointer_t*>(memory)), m_end(static_cast<Pointer_t*>(memory) + capacity), m_head(m_beg)
		{
			new (memory) Pointer_t[capacity]{};

			for (Pointer_t* ptr = m_beg; ptr <= m_end;)
			{
				Pointer_t* prev = ptr++;
				(*prev).pointer = reinterpret_cast<Stored_t*>(ptr);
			}
		}
		
		Pointer_t* m_beg;
		Pointer_t* m_end;
		Pointer_t *m_head;
	};

	template <typename Stored_t>
	FreeList<Stored_t> allocFreeList(memory::MemoryManager& memory_manager, memory::AllocationType alloc_type)
	{
		FreeList<Stored_t>* freeList = memory_manager.allocate(alloc_type);
	}

}
}
