#define APEX_ENABLE_MEMORY_LITERALS
#include "Memory/MemoryManager.h"
#include "Memory/MemoryManagerImpl.h"
#include "Memory/MemoryPool.h"
#include "Core/Asserts.h"

#include <optional>
#include <ranges>

namespace apex {
namespace mem {

	using namespace literals;

	PoolAllocator& MemoryManagerImpl::getMemoryPoolForSize(size_t allocSize)
	{
		u32 i = 0;
		for (const auto elemSize : g_memoryPoolSizes | std::views::keys)
		{
			if (allocSize <= elemSize)
				break;
			i++;
		}
		axAssert(i < m_poolAllocators.size());
		return m_poolAllocators[i];
	}

	PoolAllocator& MemoryManagerImpl::getMemoryPoolFromPointer(void* mem)
	{
		u32 i = 0;
		for ([[maybe_unused]] const auto elemSize : g_memoryPoolSizes | std::views::keys)
		{
			if (m_poolAllocators[i].containsPointer(mem))
				break;
			i++;
		}
		axAssert(i < m_poolAllocators.size());
		return m_poolAllocators[i];
	}

	bool MemoryManagerImpl::checkManaged(void* mem) const
	{
		return mem >= m_pBase && mem <= m_pBase + m_poolMemorySize;
	}

	bool MemoryManagerImpl::canFree(void* mem)
	{
		if (mem == nullptr)
			return false;

		PoolAllocator& pool = getMemoryPoolFromPointer(mem);
		const ptrdiff_t relMemPos = reinterpret_cast<ptrdiff_t>(mem) - reinterpret_cast<ptrdiff_t>(pool.m_basePtr);
		return relMemPos % pool.m_blockSize == 0;
	}

	size_t MemoryManagerImpl::getAllocatedSizeInPools() const
	{
		size_t allocated = 0;
		for (auto& poolAllocator : m_poolAllocators)
		{
			allocated += static_cast<size_t>(poolAllocator.getBlockSize()) * (poolAllocator.getTotalBlocks() - poolAllocator.getFreeBlocks());
		}
		return allocated;
	}

	void MemoryManagerImpl::setUpMemoryPools()
	{
		u32 i = 0;
		u8 *pMemItr = m_pBase; // malloc'd memory is 16 byte aligned

		for (const auto& [elemSize, poolSize] : g_memoryPoolSizes)
		{
			m_poolAllocators[i].initialize(pMemItr, static_cast<size_t>(elemSize) * poolSize, elemSize);

			pMemItr += static_cast<u64>(elemSize) * poolSize;
			i++;
		}
	}

	void MemoryManagerImpl::setUpMemoryArenas(u32 numFramesInFlight, u32 frameArenaSize)
	{
		u8 *pMemItr = m_pBase;
		pMemItr += m_poolMemorySize;

		for (u32 i = 0; i < numFramesInFlight; i++)
		{
			void* pFrameBase = detail::align_ptr(pMemItr, alignof(size_t));
			m_arenaAllocators[i].initialize(pFrameBase, frameArenaSize);
			pMemItr = static_cast<u8*>(pFrameBase) + frameArenaSize;
		}
	}

	std::pair<u32, void*> MemoryManagerImpl::allocateOnMemoryPool(size_t allocSize)
	{
		u32 poolIdx = 0;
		for (const auto elemSize : g_memoryPoolSizes | std::views::keys)
		{
			if (allocSize <= elemSize)
				break;
			poolIdx++;
		}
		axAssert(poolIdx < m_poolAllocators.size());

		void* mem = m_poolAllocators[poolIdx].allocate(allocSize);

		return { poolIdx, mem };
	}

	void MemoryManagerImpl::freeFromMemoryPool(void* mem)
	{
		if (mem == nullptr)
			return;
		getMemoryPoolFromPointer(mem).free(mem);
	}

	void MemoryManagerImpl::freeFromMemoryPool(u32 poolIdx, void* mem)
	{
		axAssert(poolIdx < m_poolAllocators.size());

		m_poolAllocators[poolIdx].free(mem);
	}

	// Memory Manager

	namespace
	{
		MemoryManagerImpl s_MemoryManagerImpl;
	}

	namespace detail
	{
		static constexpr size_t calculatePoolSizeRequirements();
	}


	void MemoryManager::initialize(MemoryManagerDesc desc)
	{
		axLog("MemoryManager initializing...");
		axAssertFmt((desc.frameArenaSize & (desc.frameArenaSize - 1)) == 0, "Frame allocator capacity must be power of 2!");

		const size_t numArenas = static_cast<size_t>(desc.numFramesInFlight) * static_cast<size_t>(MemoryTag::COUNT);
		s_MemoryManagerImpl.m_arenaAllocators.resize(numArenas);
		s_MemoryManagerImpl.m_arenaMemorySize = numArenas * desc.frameArenaSize;

		constexpr size_t numPools = std::size(g_memoryPoolSizes);
		s_MemoryManagerImpl.m_poolMemorySize = detail::calculatePoolSizeRequirements();
		s_MemoryManagerImpl.m_poolAllocators.resize(numPools);

		s_MemoryManagerImpl.m_capacity = s_MemoryManagerImpl.m_arenaMemorySize + s_MemoryManagerImpl.m_poolMemorySize;
		s_MemoryManagerImpl.m_pBase = static_cast<u8*>(::malloc(s_MemoryManagerImpl.m_capacity));

		s_MemoryManagerImpl.setUpMemoryPools();
		s_MemoryManagerImpl.setUpMemoryArenas(desc.numFramesInFlight, desc.frameArenaSize);
		axLog("MemoryManager initialized successfully");
	}

	void MemoryManager::shutdown()
	{
		axLog("MemoryManager shutting down...");
		for (ArenaAllocator& frameAllocator : s_MemoryManagerImpl.m_arenaAllocators)
		{
			frameAllocator.reset();
		}

		for (PoolAllocator& poolAllocator : s_MemoryManagerImpl.m_poolAllocators)
		{
			poolAllocator.shutdown();
		}

		s_MemoryManagerImpl.m_capacity = 0;
		::free(s_MemoryManagerImpl.m_pBase);
		s_MemoryManagerImpl.m_pBase = nullptr;
		axLog("MemoryManager shut down succesfully");
	}

	void* MemoryManager::allocate(size_t size)
	{
		return allocate(&size);
	}

	void* MemoryManager::allocate(size_t* size)
	{
		auto [poolIdx, ptr] = s_MemoryManagerImpl.allocateOnMemoryPool(*size);
		*size = s_MemoryManagerImpl.m_poolAllocators[poolIdx].getBlockSize();
		return ptr;
	}

	void MemoryManager::free(void* mem)
	{
		s_MemoryManagerImpl.freeFromMemoryPool(mem);
	}

	bool MemoryManager::checkManaged(void* mem)
	{
		return s_MemoryManagerImpl.checkManaged(mem);
	}

	bool MemoryManager::canFree(void* mem)
	{
		return s_MemoryManagerImpl.canFree(mem);
	}

	size_t MemoryManager::getTotalCapacity()
	{
		return s_MemoryManagerImpl.m_capacity;
	}

	size_t MemoryManager::getAllocatedSize()
	{
		return s_MemoryManagerImpl.getAllocatedSizeInPools();
	}

#if defined(APEX_CONFIG_DEBUG)
	MemoryManagerImpl& MemoryManager::getImplInstance()
	{
		return s_MemoryManagerImpl;
	}
#endif

	constexpr size_t detail::calculatePoolSizeRequirements()
	{
		size_t totalSize = 0;
		for (auto [elemSize, poolSize] : g_memoryPoolSizes)
		{
			totalSize += elemSize * poolSize;
		}
		return totalSize;
	}

}

	//void MemoryStats::addAllocationInfo(size_t size)
	//{
	//	m_currentUsage += size;
	//	++m_numAllocationsInFrame;
	//	++m_numAllocations;
	//	m_maxUsageInFrame = apex::max(m_maxUsageInFrame, m_currentUsage);
	//}

	//void MemoryStats::addFreeInfo(size_t size)
	//{
	//	m_currentUsage -= size;
	//	++m_numFreesInFrame;
	//	++m_numFrees;
	//}

	//void MemoryStats::beginNewFrame()
	//{
	//	m_numAllocationsInFrame = 0;
	//	m_numFreesInFrame = 0;
	//}

}
