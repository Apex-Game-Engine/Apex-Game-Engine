#pragma once

namespace apex {
namespace mem {

	struct MemoryStats
	{
		size_t m_currentUsage;
		// Last frame statistics
		size_t m_numAllocationsInFrame{};
		size_t m_numFreesInFrame{};
		size_t m_maxUsageInFrame{};
		// Global statistics
		size_t m_numAllocations{};
		size_t m_numFrees{};
		size_t m_maxUsage{};
		// Calculated statistics
		float m_averageUsage{};

		void addAllocationInfo(size_t size);
		void addFreeInfo(size_t size);
		void beginNewFrame();

		[[nodiscard]] size_t getNumMemoryAllocationsInFrame() const { return m_numAllocationsInFrame; }
		[[nodiscard]] size_t getNumMemoryFreesInFrame() const { return m_numFreesInFrame; }
		[[nodiscard]] size_t getMaxUsageInFrame() const { return m_maxUsageInFrame; }

		[[nodiscard]] size_t getNumMemoryAllocations() const { return m_numAllocations; }
		[[nodiscard]] size_t getNumMemoryFrees() const { return m_numFrees; }
		[[nodiscard]] size_t getMaxUsage() const { return m_maxUsage; }

		[[nodiscard]] float  getAverageUsage() const { return m_averageUsage; }
	};

}
}
