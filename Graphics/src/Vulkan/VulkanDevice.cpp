#include "Graphics/Vulkan/VulkanDevice.h"

#include <vulkan/vulkan.h>

#include "Graphics/Vulkan/VulkanCommon.h"
#include "Graphics/Vulkan/VulkanConfig.h"
#include "Graphics/Vulkan/VulkanFunctions.h"
#include "Graphics/Vulkan/VulkanSwapchain.h"
#include "Graphics/Vulkan/VulkanUtility.h"

char SBUF[1024];

namespace apex::vk {

	namespace detail {
		auto check_device_properties_support(VkPhysicalDeviceProperties const& device_properties) -> bool;
		auto check_device_features_support(VkPhysicalDeviceFeatures const& device_features, VkPhysicalDeviceFeatures const& required_features) -> bool;
		auto check_device_extension_support(VkPhysicalDevice device, const char* required_extensions[], size_t required_extension_count) -> bool;
		auto find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface) -> VulkanQueueFamilyIndices;
		void find_unique_queue_families(VulkanQueueFamilyIndices const& queue_family_indices, AxArray<uint32>& out_unique_queue_family_indices);
	}

	void VulkanDevice::selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDeviceFeatures const& required_features)
	{
		uint32 deviceCount;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		axAssertMsg(deviceCount > 0, "Failed to find GPUs with Vulkan support!");

		AxArray<VkPhysicalDevice> devices(deviceCount);
		devices.resize(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto& device : devices)
		{
			if (isPhysicalDeviceSuitable(device, surface, required_features))
			{
				physicalDevice = device;
				break;
			}
		}

		axAssertMsg(VK_NULL_HANDLE != physicalDevice, "Failed to find a suitable GPU!");

		// Store the physical device's properties
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

		// Store the physical device's features
		vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

		// Store the required device features that should be enabled
		requiredDeviceFeatures = required_features;

		// Store the physical device's memory properties
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

		// Store the physical device's extensions
		uint32 extensionCount;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
		availableExtensions.resize(extensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

		// Store the required queue families' indices
		queueFamilyIndices = detail::find_queue_families(physicalDevice, surface);
		hasDedicatedTransferQueue = queueFamilyIndices.transferFamily.has_value();
		if (!hasDedicatedTransferQueue)
		{
			queueFamilyIndices.transferFamily = queueFamilyIndices.graphicsFamily.value();
		}

		axAssert(hasDedicatedTransferQueue);
	}

	void VulkanDevice::createLogicalDevice(VkAllocationCallbacks const* pAllocator)
	{
		// Select unique queue families that encompass all the required operations
		AxArray<uint32> uniqueQueueFamilies;
		detail::find_unique_queue_families(queueFamilyIndices, uniqueQueueFamilies);

		// Create a queue for each of the required queue families
		AxArray<VkDeviceQueueCreateInfo> queueCreateInfos(uniqueQueueFamilies.size());
		float queuePriority = 1.f;
		for (uint32 queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo {
				.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = queueFamily,
				.queueCount       = 1,
				.pQueuePriorities = &queuePriority
			};

			queueCreateInfos.append(queueCreateInfo);
		}

		VkDeviceCreateInfo createInfo {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.queueCreateInfoCount = static_cast<uint32>(queueCreateInfos.size()),
			.pQueueCreateInfos = queueCreateInfos.data(),
			.enabledLayerCount = static_cast<uint32>(std::size(VulkanConfig::kValidationLayerNames)),
			.ppEnabledLayerNames = VulkanConfig::kValidationLayerNames,
			.enabledExtensionCount = static_cast<uint32>(std::size(VulkanConfig::kRequiredDeviceExtensions)),
			.ppEnabledExtensionNames = VulkanConfig::kRequiredDeviceExtensions,
			.pEnabledFeatures = &requiredDeviceFeatures
		};

		axVerifyMsg(VK_SUCCESS == vkCreateDevice(physicalDevice, &createInfo, pAllocator, &logicalDevice),
		            "Failed to create logical device!"
		);

		// Store handle to the queues
		vkGetDeviceQueue(logicalDevice, queueFamilyIndices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(logicalDevice, queueFamilyIndices.presentFamily.value(), 0, &presentQueue);
		if (queueFamilyIndices.transferFamily.has_value())
		{
			vkGetDeviceQueue(logicalDevice, queueFamilyIndices.transferFamily.value(), 0, &transferQueue);
		}
		else
		{
			axWarn("Failed to find dedicated Transfer queue family!");
		}

		if (queueFamilyIndices.computeFamily.has_value())
		{
			vkGetDeviceQueue(logicalDevice, queueFamilyIndices.computeFamily.value(), 0, &computeQueue);
		}
		else
		{
			axWarn("Failed to find dedicated Compute queue family!");
		}
	}

	void VulkanDevice::createCommandPools(VkAllocationCallbacks const* pAllocator)
	{
		// Create a command pool for transfer operations
		VkCommandPoolCreateInfo transferCommandPoolCreateInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
			.queueFamilyIndex = queueFamilyIndices.transferFamily.value(),
		};

		axVerifyMsg(VK_SUCCESS == vkCreateCommandPool(logicalDevice, &transferCommandPoolCreateInfo, pAllocator, &transferCommandPool),
			"Failed to create transfer command pool!"
		);
	}

	void VulkanDevice::destroy(VkAllocationCallbacks const* pAllocator)
	{
		vkDestroyCommandPool(logicalDevice, transferCommandPool, pAllocator);
		vkDestroyDevice(logicalDevice, pAllocator);
	}

	auto VulkanDevice::beginOneShotCommandBuffer(VkCommandPool command_pool) const -> VkCommandBuffer
	{
		VkCommandBufferAllocateInfo commandBufferAllocateInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = command_pool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1
		};

		VkCommandBuffer commandBuffer{};

		axVerifyMsg(VK_SUCCESS == vkAllocateCommandBuffers(logicalDevice, &commandBufferAllocateInfo, &commandBuffer),
			"Failed to allocate command buffer!"
		);

		VkDebugUtilsObjectNameInfoEXT commandBufferNameInfo {
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
			.objectType = VK_OBJECT_TYPE_COMMAND_BUFFER,
			.objectHandle = reinterpret_cast<uint64>(commandBuffer),
			.pObjectName = "One-shot Command Buffer",
		};

		vk::SetDebugUtilsObjectNameEXT(logicalDevice, &commandBufferNameInfo);

		VkCommandBufferBeginInfo commandBufferBeginInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
		};

		vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

		return commandBuffer;
	}

	// TODO: Make return type std::optional
	auto VulkanDevice::findSuitableMemoryType(uint32 type_filter, VkMemoryPropertyFlags properties) const -> uint32
	{
		for (uint32 i = 0; i < deviceMemoryProperties.memoryTypeCount; i++)
		{
			if (type_filter & (1 << i) && (deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		axAssertMsg(false, "Failed to find suitable memory type!");

		return constants::uint32_MAX;
	}

	void VulkanDevice::createMemoryAllocator(VkInstance instance, VkAllocationCallbacks const* pAllocator)
	{
		VmaAllocatorCreateInfo allocatorCreateInfo {
			.flags = 0,
			.physicalDevice = physicalDevice,
			.device = logicalDevice,
			.instance = instance,
		};
		axAssertMsg(VK_SUCCESS == vmaCreateAllocator(&allocatorCreateInfo, &m_allocator),
			"Failed to create Vulkan Memory Allocator!"
		);
	}

	bool VulkanDevice::isPhysicalDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, VkPhysicalDeviceFeatures required_features)
	{
		// Check if required device properties are present
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(device, &properties);

		sprintf_s(SBUF, "Found GPU: %s", properties.deviceName);
		axDebug(SBUF);

		if (!detail::check_device_properties_support(properties))
		{
			return false;
		}

		// Check if required device features are present
		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(device, &features);
		if (!detail::check_device_features_support(features, required_features))
		{
			return false;
		}

		// Check if required queue families are present
		VulkanQueueFamilyIndices queueFamilies = detail::find_queue_families(device, surface);
		if (!queueFamilies.isRenderComplete())
		{
			return false;
		}

		// Check if required extensions are supported
		bool extensionsSupported = detail::check_device_extension_support(device, VulkanConfig::kRequiredDeviceExtensions, std::size(VulkanConfig::kRequiredDeviceExtensions));
		if (!extensionsSupported)
		{
			return false;
		}

		// Check if the swap chain support is adequate
		VulkanSwapchainSupportDetails swapchainSupport = query_swapchain_support_details(device, surface);
		if (swapchainSupport.formats.empty() || swapchainSupport.presentModes.empty())
		{
			return false;
		}

		return true;
	}

	bool detail::check_device_properties_support(VkPhysicalDeviceProperties const& device_properties)
	{
		return VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU == device_properties.deviceType;
	}

	bool detail::check_device_features_support(VkPhysicalDeviceFeatures const& device_features, VkPhysicalDeviceFeatures const& required_features)
	{
		return
			(!required_features.robustBufferAccess || device_features.robustBufferAccess)
			&& (!required_features.fullDrawIndexUint32 || device_features.fullDrawIndexUint32)
			&& (!required_features.imageCubeArray || device_features.imageCubeArray)
			&& (!required_features.independentBlend || device_features.independentBlend)
			&& (!required_features.geometryShader || device_features.geometryShader)
			&& (!required_features.tessellationShader || device_features.tessellationShader)
			&& (!required_features.sampleRateShading || device_features.sampleRateShading)
			&& (!required_features.dualSrcBlend || device_features.dualSrcBlend)
			&& (!required_features.logicOp || device_features.logicOp)
			&& (!required_features.multiDrawIndirect || device_features.multiDrawIndirect)
			&& (!required_features.drawIndirectFirstInstance || device_features.drawIndirectFirstInstance)
			&& (!required_features.depthClamp || device_features.depthClamp)
			&& (!required_features.depthBiasClamp || device_features.depthBiasClamp)
			&& (!required_features.fillModeNonSolid || device_features.fillModeNonSolid)
			&& (!required_features.depthBounds || device_features.depthBounds)
			&& (!required_features.wideLines || device_features.wideLines)
			&& (!required_features.largePoints || device_features.largePoints)
			&& (!required_features.alphaToOne || device_features.alphaToOne)
			&& (!required_features.multiViewport || device_features.multiViewport)
			&& (!required_features.samplerAnisotropy || device_features.samplerAnisotropy)
			&& (!required_features.textureCompressionETC2 || device_features.textureCompressionETC2)
			&& (!required_features.textureCompressionASTC_LDR || device_features.textureCompressionASTC_LDR)
			&& (!required_features.textureCompressionBC || device_features.textureCompressionBC)
			&& (!required_features.occlusionQueryPrecise || device_features.occlusionQueryPrecise)
			&& (!required_features.pipelineStatisticsQuery || device_features.pipelineStatisticsQuery)
			&& (!required_features.vertexPipelineStoresAndAtomics || device_features.vertexPipelineStoresAndAtomics)
			&& (!required_features.fragmentStoresAndAtomics || device_features.fragmentStoresAndAtomics)
			&& (!required_features.shaderTessellationAndGeometryPointSize || device_features.shaderTessellationAndGeometryPointSize)
			&& (!required_features.shaderImageGatherExtended || device_features.shaderImageGatherExtended)
			&& (!required_features.shaderStorageImageExtendedFormats || device_features.shaderStorageImageExtendedFormats)
			&& (!required_features.shaderStorageImageMultisample || device_features.shaderStorageImageMultisample)
			&& (!required_features.shaderStorageImageReadWithoutFormat || device_features.shaderStorageImageReadWithoutFormat)
			&& (!required_features.shaderStorageImageWriteWithoutFormat || device_features.shaderStorageImageWriteWithoutFormat)
			&& (!required_features.shaderUniformBufferArrayDynamicIndexing || device_features.shaderUniformBufferArrayDynamicIndexing)
			&& (!required_features.shaderSampledImageArrayDynamicIndexing || device_features.shaderSampledImageArrayDynamicIndexing)
			&& (!required_features.shaderStorageBufferArrayDynamicIndexing || device_features.shaderStorageBufferArrayDynamicIndexing)
			&& (!required_features.shaderStorageImageArrayDynamicIndexing || device_features.shaderStorageImageArrayDynamicIndexing)
			&& (!required_features.shaderClipDistance || device_features.shaderClipDistance)
			&& (!required_features.shaderCullDistance || device_features.shaderCullDistance)
			&& (!required_features.shaderFloat64 || device_features.shaderFloat64)
			&& (!required_features.shaderInt64 || device_features.shaderInt64)
			&& (!required_features.shaderInt16 || device_features.shaderInt16)
			&& (!required_features.shaderResourceResidency || device_features.shaderResourceResidency)
			&& (!required_features.shaderResourceMinLod || device_features.shaderResourceMinLod)
			&& (!required_features.sparseBinding || device_features.sparseBinding)
			&& (!required_features.sparseResidencyBuffer || device_features.sparseResidencyBuffer)
			&& (!required_features.sparseResidencyImage2D || device_features.sparseResidencyImage2D)
			&& (!required_features.sparseResidencyImage3D || device_features.sparseResidencyImage3D)
			&& (!required_features.sparseResidency2Samples || device_features.sparseResidency2Samples)
			&& (!required_features.sparseResidency4Samples || device_features.sparseResidency4Samples)
			&& (!required_features.sparseResidency8Samples || device_features.sparseResidency8Samples)
			&& (!required_features.sparseResidency16Samples || device_features.sparseResidency16Samples)
			&& (!required_features.sparseResidencyAliased || device_features.sparseResidencyAliased)
			&& (!required_features.variableMultisampleRate || device_features.variableMultisampleRate)
			&& (!required_features.inheritedQueries || device_features.inheritedQueries);
	}

	auto detail::find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface) -> VulkanQueueFamilyIndices
	{
		VulkanQueueFamilyIndices indices{};

		uint32 queueFamiliesCount;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount, nullptr);

		AxArray<VkQueueFamilyProperties> queueFamilies;
		queueFamilies.resize(queueFamiliesCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount, queueFamilies.data());

		int i = 0;
		for (const VkQueueFamilyProperties& queueFamily : queueFamilies)
		{
			// Find a queue family that supports Graphics operations
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			// Find a queue family that supports presenting to the window surface
			VkBool32 presentSupport;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport)
			{
				indices.presentFamily = i;
			}

			// Find a dedicated compute queue family
			if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				indices.computeFamily = i;
			}

			// Find a dedicated transfer queue family
			if ((queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
				&& !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				&& !(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT))
			{
				indices.transferFamily = i;
			}

			if (indices.isComplete())
			{
				break;
			}
			++i;
		}

		return indices;
	}

	void detail::find_unique_queue_families(VulkanQueueFamilyIndices const& queue_family_indices, AxArray<uint32>& out_unique_queue_family_indices)
	{
		out_unique_queue_family_indices.reserve(4);

		out_unique_queue_family_indices.append(queue_family_indices.graphicsFamily.value());
		out_unique_queue_family_indices.append(queue_family_indices.presentFamily.value());
		if (queue_family_indices.transferFamily.has_value())
		{
			out_unique_queue_family_indices.append(queue_family_indices.transferFamily.value());
		}
		if (queue_family_indices.computeFamily.has_value())
		{
			out_unique_queue_family_indices.append(queue_family_indices.computeFamily.value());
		}

		apex::keepUniquesOnly_slow(out_unique_queue_family_indices);
	}

	auto detail::check_device_extension_support(VkPhysicalDevice device, const char* required_extensions[], size_t required_extension_count) -> bool
	{
		// List all the available extensions on the physical device
		uint32 extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		AxArray<VkExtensionProperties> availableExtensions(extensionCount);
		availableExtensions.resize(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		// Check if the required extensions are all available on the physical device
		AxArray<bool> extensionsPresent(extensionCount);
		extensionsPresent.resize(extensionCount, false);

		size_t numExtensionsPresent = 0;

		for (const VkExtensionProperties& extension : availableExtensions)
		{
			for (size_t i = 0; i < required_extension_count; i++)
			{
				if (!extensionsPresent[i] && strcmp(extension.extensionName, required_extensions[i]) == 0)
				{
					extensionsPresent[i] = true;
					numExtensionsPresent++;
				}
			}
			if (numExtensionsPresent == required_extension_count)
				break;
		}

		return numExtensionsPresent == required_extension_count;
	}

}
