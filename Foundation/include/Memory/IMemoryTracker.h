#pragma once

namespace apex {
namespace memory {

	class IMemoryTracker
	{
	public:
		virtual ~IMemoryTracker() = default;

		[[nodiscard]] virtual size_t getTotalCapacity() const = 0;
		[[nodiscard]] virtual size_t getCurrentUsage() const = 0;
	};

	void DebugMemoryTrackingInfo(IMemoryTracker* tracker);
	/*{
		float32 percentUsage = static_cast<float32>(tracker->getCurrentUsage()) / static_cast<float32>(tracker->getTotalCapacity());

		char buf[1024];
		sprintf_s(buf, "Memory Usage:\n\tUsed memory: %lld bytes\n\t%age used memory: %f", tracker->getCurrentUsage(), percentUsage);
		axDebug(buf);
	}*/

}
}
