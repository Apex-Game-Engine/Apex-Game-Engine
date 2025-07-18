#include "Graphics/Vulkan/VulkanContext.h"

#include <vulkan/vk_enum_string_helper.h>

#include "Platform/PlatformManager.h"
#include "Core/Logging.h"
#include "Core/Asserts.h"
#include "Core/Files.h"
#include "Graphics/Factory.h"
#include "Math/Vector4.h"
#include "Memory/MemoryManager.h"
#include "Memory/UniquePtr.h"

namespace apex::gfx {

	struct VulkanDebugUtils
	{
		PFN_vkCreateDebugUtilsMessengerEXT		CreateDebugUtilsMessenger {};
		PFN_vkDestroyDebugUtilsMessengerEXT		DestroyDebugUtilsMessenger {};
		PFN_vkSetDebugUtilsObjectNameEXT		SetDebugUtilsObjectName {};
		PFN_vkQueueBeginDebugUtilsLabelEXT		QueueBeginDebugUtilsLabel {};
		PFN_vkQueueEndDebugUtilsLabelEXT		QueueEndDebugUtilsLabel {};
		PFN_vkCmdBeginDebugUtilsLabelEXT		CmdBeginDebugUtilsLabel {};
		PFN_vkCmdEndDebugUtilsLabelEXT			CmdEndDebugUtilsLabel {};
	};

	static VulkanDebugUtils vkdbg;

	struct VulkanSwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities {};
		AxArray<VkSurfaceFormatKHR> formats;
		AxArray<VkPresentModeKHR> presentModes;
	};

	class VulkanContextImpl
	{
		friend class VulkanContext;

	public:
	#if APEX_PLATFORM_WIN32
		VulkanContextImpl(HINSTANCE hinstance, HWND hwnd);
	#else
		VulkanContextImpl();
	#endif
		~VulkanContextImpl();

	#if APEX_PLATFORM_WIN32
		void CreateSurface(HINSTANCE hinstance, HWND hwnd);
	#endif
		VkPhysicalDevice SelectPhysicalDevice(VulkanPhysicalDeviceFeatures const& required_device_features) const;
		void CreateDevice(VkPhysicalDevice physical_device, VulkanPhysicalDeviceFeatures const& enabled_device_features);
		bool ResizeSurface();

		bool ResizeSurface(u32 width, u32 height) const;

		VkInstance					GetInstance() const		{ return m_instance; }
		VkSurfaceKHR				GetSurface() const		{ return m_surface; }
		VulkanDevice const&			GetDevice() const		{ return *m_device; }

	private:
		VkInstance					m_instance {};
		VkSurfaceKHR				m_surface {};
		UniquePtr<VulkanDevice>		m_device {};
		VkDebugUtilsMessengerEXT	m_debugUtilsMessenger {};
	};

	static VkBool32 DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
		VkDebugUtilsMessageTypeFlagsEXT message_type,
		const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data)
	{
		switch (message_severity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			{
				axDebugFmt("[{}] {} : {}", p_callback_data->messageIdNumber, p_callback_data->pMessageIdName, p_callback_data->pMessage);
				break;
			}
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			{
				axInfoFmt("[{}] {} : {}", p_callback_data->messageIdNumber, p_callback_data->pMessageIdName, p_callback_data->pMessage);
				break;
			}
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			{
				axWarnFmt("[{}] {} : {}", p_callback_data->messageIdNumber, p_callback_data->pMessageIdName, p_callback_data->pMessage);
				break;
			}
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			{
				axErrorFmt("[{}] {} : {}", p_callback_data->messageIdNumber, p_callback_data->pMessageIdName, p_callback_data->pMessage);
				break;
			}
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT: break;
		}
		return VK_FALSE;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
		VkDebugUtilsMessageTypeFlagsEXT             message_types,
		const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
		void*                                       p_user_data)
	{
		return DebugCallback(message_severity, message_types, p_callback_data);
	}

	static bool CheckValidationLayerSupport(const char* validation_layer_names[], size_t validation_layer_count)
	{
		u32 instanceLayerCount;
		vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);

		AxArray<VkLayerProperties> instanceLayerProperties;
		instanceLayerProperties.resize(instanceLayerCount);
		vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());

		AxArray<bool> validationLayersPresent;
		validationLayersPresent.resize(instanceLayerCount, false);

		size_t numValidationLayersPresent = 0;

		for (VkLayerProperties& layerProperties : instanceLayerProperties)
		{
			for (size_t i = 0; i < validation_layer_count; i++)
			{
				if (!validationLayersPresent[i] && strcmp(layerProperties.layerName, validation_layer_names[i]) == 0)
				{
					validationLayersPresent[i] = true;
					numValidationLayersPresent++;
				}
			}
		}
		return numValidationLayersPresent == validation_layer_count;
	}

	static bool CheckPhysicalDeviceProperties(VulkanPhysicalDeviceProperties const& device_properties)
	{
		// TODO: Add checks for important device properties
		return device_properties.properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
			&& device_properties.properties.properties.apiVersion >= VK_API_VERSION_1_3;
	}

	template <typename Features>
	AxArrayRef<const VkBool32> GetFeaturesArray(Features const& features)
	{
		const size_t offset = offsetof(Features, pNext) + sizeof(void*);
		return { ._data = reinterpret_cast<const VkBool32*>(reinterpret_cast<const char*>(&features) + offset), .count = (sizeof(Features) - offset) / sizeof(VkBool32) };
	}

	template <>
	AxArrayRef<const VkBool32> GetFeaturesArray(VkPhysicalDeviceFeatures const& features)
	{
		return { ._data = &features.robustBufferAccess, .count = (sizeof(VkPhysicalDeviceFeatures) - offsetof(VkPhysicalDeviceFeatures, robustBufferAccess)) / sizeof(VkBool32) };
	}

	static bool CheckFeaturesArray(AxArrayRef<const VkBool32> required, AxArrayRef<const VkBool32> actual)
	{
		for (size_t i = 0; i < required.count; i++)
		{
			if (required[i] && !actual[i])
			{
				return false;
			}
		}
		return true;
	}

	static bool CheckPhysicalDeviceFeatures(VulkanPhysicalDeviceFeatures const& device_features, VulkanPhysicalDeviceFeatures const& required_features)
	{
		const AxArrayRef<const VkBool32> requiredFeatures10 = GetFeaturesArray(required_features.features.features);
		const AxArrayRef<const VkBool32> requiredFeatures11 = GetFeaturesArray(required_features.features11);
		const AxArrayRef<const VkBool32> requiredFeatures12 = GetFeaturesArray(required_features.features12);
		const AxArrayRef<const VkBool32> requiredFeatures13 = GetFeaturesArray(required_features.features13);

		const AxArrayRef<const VkBool32> deviceFeatures10 = GetFeaturesArray(device_features.features.features);
		const AxArrayRef<const VkBool32> deviceFeatures11 = GetFeaturesArray(device_features.features11);
		const AxArrayRef<const VkBool32> deviceFeatures12 = GetFeaturesArray(device_features.features12);
		const AxArrayRef<const VkBool32> deviceFeatures13 = GetFeaturesArray(device_features.features13);

		return CheckFeaturesArray(requiredFeatures10, deviceFeatures10)
			&& CheckFeaturesArray(requiredFeatures11, deviceFeatures11)
			&& CheckFeaturesArray(requiredFeatures12, deviceFeatures12)
			&& CheckFeaturesArray(requiredFeatures13, deviceFeatures13)
		;
	}

	template <typename Tin>
	static const void* FindInStructureChain(VkStructureType sType, Tin const* in)
	{
		while (in)
		{
			if (in->sType == sType)
			{
				break;
			}
			in = static_cast<Tin const*>(in->pNext);
		}
		return in;
	}

	static bool IsPhysicalDeviceSuitable(VkPhysicalDevice physical_device, VkSurfaceKHR surface, VulkanPhysicalDeviceFeatures const& required_features)
	{
		VulkanPhysicalDeviceProperties properties;
		properties.SetupChain(nullptr);
		vkGetPhysicalDeviceProperties2(physical_device, &properties.properties);

		axDebugFmt("Found GPU: {}", properties.properties.properties.deviceName);

		if (!CheckPhysicalDeviceProperties(properties))
		{
			return false;
		}

		VulkanPhysicalDeviceFeatures deviceFeatures;
		VkPhysicalDeviceShaderAtomicFloatFeaturesEXT atomicFloatFeatures {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT,
			.pNext = nullptr,
		};
		deviceFeatures.SetupChain(&atomicFloatFeatures);
		vkGetPhysicalDeviceFeatures2(physical_device, &deviceFeatures.features);
		if (!CheckPhysicalDeviceFeatures(deviceFeatures, required_features))
		{
			return false;
		}
		if (VK_TRUE != atomicFloatFeatures.shaderBufferFloat32AtomicAdd)
		{
			return false;
		}

		return true;
	}

	static void FindQueueFamilies(VkPhysicalDevice physical_device, VkSurfaceKHR surface, bool use_dedicated_transfer_queue, VulkanQueueInfo* queueInfos)
	{
		u32 queueFamiliesCount;
		vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, &queueFamiliesCount, nullptr);

		AxArray<VkQueueFamilyProperties2> queueFamilies;
		queueFamilies.resize(queueFamiliesCount, VkQueueFamilyProperties2{ .sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2, .pNext = nullptr });
		vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, &queueFamiliesCount, queueFamilies.dataMutable());

		VulkanQueueInfo& graphicsQueue = queueInfos[QueueType::Graphics];
		VulkanQueueInfo& computeQueue  = queueInfos[QueueType::Compute];
		VulkanQueueInfo& transferQueue = queueInfos[QueueType::Transfer];

		graphicsQueue.familyIndex = -1;
		computeQueue.familyIndex = -1;
		transferQueue.familyIndex = -1;
		VkBool32 presentSupport = VK_FALSE;

		s32 i = 0;
		for (const VkQueueFamilyProperties2& queueFamily : queueFamilies)
		{
			VkQueueFlags flags = queueFamily.queueFamilyProperties.queueFlags;
			if (graphicsQueue.familyIndex == VulkanQueueInfo::kInvalidQueueFamilyIndex && (flags & VK_QUEUE_GRAPHICS_BIT))
			{
				graphicsQueue.familyIndex = i;
				graphicsQueue.supportsGraphics = true;
				(void)(presentSupport || vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &presentSupport));
				if (presentSupport)
				{
					graphicsQueue.supportsPresent = true;
				}
			}
			else if (computeQueue.familyIndex == VulkanQueueInfo::kInvalidQueueFamilyIndex && (flags & VK_QUEUE_COMPUTE_BIT))
			{
				computeQueue.familyIndex  = i;
				computeQueue.supportsCompute = true;
			}
			else if (transferQueue.familyIndex == VulkanQueueInfo::kInvalidQueueFamilyIndex && (flags & VK_QUEUE_TRANSFER_BIT) && (!use_dedicated_transfer_queue || (!(flags & VK_QUEUE_GRAPHICS_BIT) && !(flags & VK_QUEUE_COMPUTE_BIT))))
			{
				transferQueue.familyIndex = i;
				transferQueue.supportsTransfer = true;
			}

			if (graphicsQueue.familyIndex != VulkanQueueInfo::kInvalidQueueFamilyIndex && 
				computeQueue.familyIndex != VulkanQueueInfo::kInvalidQueueFamilyIndex && 
				transferQueue.familyIndex != VulkanQueueInfo::kInvalidQueueFamilyIndex)
			{
				break;
			}

			++i;
		}
		
		axAssertFmt(presentSupport, "Graphics queue does not support Surface presentation!");

		// If dedicated transfer queue family is not found then use the graphics queue family for transfer operations
		if (transferQueue.familyIndex == VulkanQueueInfo::kInvalidQueueFamilyIndex)
		{
			graphicsQueue.supportsTransfer = true;
		}

	}

	static void QuerySwapchainSupportDetails(VkPhysicalDevice physical_device, VkSurfaceKHR surface, VulkanSwapchainSupportDetails& swapchain_support)
	{
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &swapchain_support.capabilities);

		u32 formatsCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatsCount, nullptr);
		swapchain_support.formats.resize(formatsCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatsCount, swapchain_support.formats.data());

		u32 presentModesCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentModesCount, nullptr);
		swapchain_support.presentModes.resize(presentModesCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentModesCount, swapchain_support.presentModes.dataMutable());
	}

	static void SetObjectName(VkDevice device, VkObjectType type, void* handle, const char* name)
	{
		const VkDebugUtilsObjectNameInfoEXT objectNameInfo {
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
			.objectType = type,
			.objectHandle = reinterpret_cast<u64>(handle),
			.pObjectName = name,
		};

		vkdbg.SetDebugUtilsObjectName(device, &objectNameInfo);
	}

	constexpr static VkMemoryPropertyFlags ConvertToVkMemoryPropertyFlags(MemoryPropertyFlags flags)
	{
		VkMemoryPropertyFlags vkFlags = 0;
		if (flags & MemoryPropertyFlagBits::DeviceLocal)
		{
			vkFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		}
		if (flags & MemoryPropertyFlagBits::HostVisible)
		{
			vkFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		}
		if (flags & MemoryPropertyFlagBits::HostCoherent)
		{
			vkFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		}
		if (flags & MemoryPropertyFlagBits::HostCached)
		{
			vkFlags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
		}
		if (flags & MemoryPropertyFlagBits::LazilyAllocated)
		{
			vkFlags |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
		}
		return vkFlags;
	}

	constexpr static VmaAllocationCreateFlags ConvertToVmaAllocationCreateFlags(MemoryAllocateFlags flags)
	{
		VmaAllocationCreateFlags vkFlags = 0;
		if (flags & MemoryAllocateFlagBits::AllocateDedicated)
		{
			vkFlags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		}
		if (flags & MemoryAllocateFlagBits::NeverAllocate)
		{
			vkFlags |= VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT;
		}
		if (flags & MemoryAllocateFlagBits::CanAlias)
		{
			vkFlags |= VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT;
		}
		if (flags & MemoryAllocateFlagBits::HostAccessSequential)
		{
			vkFlags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		}
		if (flags & MemoryAllocateFlagBits::HostAccessRandom)
		{
			vkFlags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
		}
		if (flags & MemoryAllocateFlagBits::HostAccessFallbackTransfer)
		{
			vkFlags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT;
		}
		return vkFlags;
	}

	constexpr static VkBufferUsageFlags ConvertToVkBufferUsageFlags(BufferUsageFlags flags)
	{
		VkBufferUsageFlags vkFlags = 0;
		if (flags & BufferUsageFlagBits::Vertex)
		{
			vkFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		}
		if (flags & BufferUsageFlagBits::Index)
		{
			vkFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		}
		if (flags & BufferUsageFlagBits::Uniform)
		{
			vkFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		}
		if (flags & BufferUsageFlagBits::Storage)
		{
			vkFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		}
		if (flags & BufferUsageFlagBits::Indirect)
		{
			vkFlags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
		}
		if (flags & BufferUsageFlagBits::TransferSrc)
		{
			vkFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		}
		if (flags & BufferUsageFlagBits::TransferDst)
		{
			vkFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}
		return vkFlags;
	}

	constexpr static VkImageUsageFlags ConvertToVkImageUsageFlags(ImageUsageFlags flags)
	{
		VkImageUsageFlags vkFlags = 0;
		if (flags & ImageUsageFlagBits::TransferSrc)
		{
			vkFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}
		if (flags & ImageUsageFlagBits::TransferDst)
		{
			vkFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}
		if (flags & ImageUsageFlagBits::Sampled)
		{
			vkFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
		}
		if (flags & ImageUsageFlagBits::Storage)
		{
			vkFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
		}
		if (flags & ImageUsageFlagBits::ColorAttachment)
		{
			vkFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}
		if (flags & ImageUsageFlagBits::DepthStencilAttachment)
		{
			vkFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}
		if (flags & ImageUsageFlagBits::TransientAttachment)
		{
			vkFlags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
		}
		if (flags & ImageUsageFlagBits::InputAttachment)
		{
			vkFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		}
		return vkFlags;
	}

	constexpr static VkImageLayout ConvertToVkImageLayout(ImageLayout layout)
	{
		switch (layout)
		{
		case ImageLayout::PresentSrcOptimal: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		default:
			return static_cast<VkImageLayout>(layout);
		}
	}

	constexpr static VkAccessFlags ConvertToVkAccessFlags(AccessFlags flags)
	{
		return static_cast<VkAccessFlags>(flags);
	}

	constexpr static VkPipelineStageFlags ConvertToVkPipelineStageFlags(PipelineStageFlags flags)
	{
		return static_cast<VkPipelineStageFlags>(flags);
	}

	constexpr static VkExtent2D ConvertToVkExtent2D(Dim2D dim)
	{
		return VkExtent2D { dim.width, dim.height };
	}

	constexpr static VkExtent3D ConvertToVkExtent3D(Dim3D dim)
	{
		return VkExtent3D { dim.width, dim.height, dim.depth };
	}

	constexpr static VkImageType ConvertToVkImageType(ImageType type)
	{
		switch (type)
		{
		case ImageType::Image1D:
		case ImageType::Image1DArray:
			return VK_IMAGE_TYPE_1D;
		case ImageType::Image2D:
		case ImageType::Image2DArray:
		case ImageType::ImageCube:
		case ImageType::ImageCubeArray:
			return VK_IMAGE_TYPE_2D;
		case ImageType::Image3D:
			return VK_IMAGE_TYPE_3D;
		}
	}

	constexpr static VkImageViewType  ConvertToVkImageViewType(ImageType type)
	{
		switch (type)
		{
		case ImageType::Image1D: return        VK_IMAGE_VIEW_TYPE_1D;
		case ImageType::Image2D: return        VK_IMAGE_VIEW_TYPE_2D;
		case ImageType::Image3D: return        VK_IMAGE_VIEW_TYPE_3D;
		case ImageType::ImageCube: return      VK_IMAGE_VIEW_TYPE_CUBE;
		case ImageType::Image1DArray: return   VK_IMAGE_VIEW_TYPE_1D_ARRAY;
		case ImageType::Image2DArray: return   VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		case ImageType::ImageCubeArray: return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
		}
	}

	constexpr static VkFormat ConvertToVkFormat(ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::R8_UNORM:				return VK_FORMAT_R8_UNORM;
		case ImageFormat::R8_SNORM:				return VK_FORMAT_R8_SNORM;
		case ImageFormat::R8_UINT:				return VK_FORMAT_R8_UINT;
		case ImageFormat::R8_SINT:				return VK_FORMAT_R8_SINT;
		case ImageFormat::R8_SRGB:				return VK_FORMAT_R8_SRGB;
		case ImageFormat::R8G8_UNORM:			return VK_FORMAT_R8G8_UNORM;
		case ImageFormat::R8G8_SNORM:			return VK_FORMAT_R8G8_SNORM;
		case ImageFormat::R8G8_UINT:			return VK_FORMAT_R8G8_UINT;
		case ImageFormat::R8G8_SINT:			return VK_FORMAT_R8G8_SINT;
		case ImageFormat::R8G8_SRGB:			return VK_FORMAT_R8G8_SRGB;
		case ImageFormat::R8G8B8_UNORM:			return VK_FORMAT_R8G8B8_UNORM;
		case ImageFormat::R8G8B8_SNORM:			return VK_FORMAT_R8G8B8_SNORM;
		case ImageFormat::R8G8B8_UINT:			return VK_FORMAT_R8G8B8_UINT;
		case ImageFormat::R8G8B8_SINT:			return VK_FORMAT_R8G8B8_SINT;
		case ImageFormat::R8G8B8_SRGB:			return VK_FORMAT_R8G8B8_SRGB;
		case ImageFormat::B8G8R8_UNORM:			return VK_FORMAT_B8G8R8_UNORM;
		case ImageFormat::B8G8R8_SNORM:			return VK_FORMAT_B8G8R8_SNORM;
		case ImageFormat::B8G8R8_UINT:			return VK_FORMAT_B8G8R8_UINT;
		case ImageFormat::B8G8R8_SINT:			return VK_FORMAT_B8G8R8_SINT;
		case ImageFormat::B8G8R8_SRGB:			return VK_FORMAT_B8G8R8_SRGB;
		case ImageFormat::R8G8B8A8_UNORM:		return VK_FORMAT_R8G8B8A8_UNORM;
		case ImageFormat::R8G8B8A8_SNORM:		return VK_FORMAT_R8G8B8A8_SNORM;
		case ImageFormat::R8G8B8A8_UINT:		return VK_FORMAT_R8G8B8A8_UINT;
		case ImageFormat::R8G8B8A8_SINT:		return VK_FORMAT_R8G8B8A8_SINT;
		case ImageFormat::R8G8B8A8_SRGB:		return VK_FORMAT_R8G8B8A8_SRGB;
		case ImageFormat::B8G8R8A8_UNORM:		return VK_FORMAT_B8G8R8A8_UNORM;
		case ImageFormat::B8G8R8A8_SNORM:		return VK_FORMAT_B8G8R8A8_SNORM;
		case ImageFormat::B8G8R8A8_UINT:		return VK_FORMAT_B8G8R8A8_UINT;
		case ImageFormat::B8G8R8A8_SINT:		return VK_FORMAT_B8G8R8A8_SINT;
		case ImageFormat::B8G8R8A8_SRGB:		return VK_FORMAT_B8G8R8A8_SRGB;
		case ImageFormat::R16_FLOAT:			return VK_FORMAT_R16_SFLOAT;
		case ImageFormat::R32_FLOAT:			return VK_FORMAT_R32_SFLOAT;
		case ImageFormat::D16_UNORM:			return VK_FORMAT_D16_UNORM;
		case ImageFormat::D32_FLOAT:			return VK_FORMAT_D32_SFLOAT;
		case ImageFormat::S8_UINT:				return VK_FORMAT_S8_UINT;
		case ImageFormat::D16_UNORM_S8_UINT:	return VK_FORMAT_D16_UNORM_S8_UINT;
		case ImageFormat::D24_UNORM_S8_UINT:	return VK_FORMAT_D24_UNORM_S8_UINT;
		case ImageFormat::UNDEFINED:      ;
		}
		return VK_FORMAT_UNDEFINED;
	}

	constexpr static VkDescriptorType ConvertToVkDescriptorType(DescriptorType type)
	{
		switch (type)
		{
		case DescriptorType::eInputAttachment:      return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		case DescriptorType::eSampler:              return VK_DESCRIPTOR_TYPE_SAMPLER;
		case DescriptorType::eCombinedImageSampler: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		case DescriptorType::eSampledImage:         return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		case DescriptorType::eStorageImage:         return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		case DescriptorType::eUniformBuffer:        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case DescriptorType::eStorageBuffer:        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		}
	}

	constexpr static u32 GetFormatSize(VkFormat format)
	{
		switch (format)
		{
		case VK_FORMAT_UNDEFINED:
			return 0;
		case VK_FORMAT_R4G4_UNORM_PACK8:
			return 1;
		case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
		case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
		case VK_FORMAT_R5G6B5_UNORM_PACK16:
		case VK_FORMAT_B5G6R5_UNORM_PACK16:
		case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
		case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
		case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
			return 2;
		case VK_FORMAT_R8_UNORM:
		case VK_FORMAT_R8_SNORM:
		case VK_FORMAT_R8_USCALED:
		case VK_FORMAT_R8_SSCALED:
		case VK_FORMAT_R8_UINT:
		case VK_FORMAT_R8_SINT:
		case VK_FORMAT_R8_SRGB:
			return 1;
		case VK_FORMAT_R8G8_UNORM:
		case VK_FORMAT_R8G8_SNORM:
		case VK_FORMAT_R8G8_USCALED:
		case VK_FORMAT_R8G8_SSCALED:
		case VK_FORMAT_R8G8_UINT:
		case VK_FORMAT_R8G8_SINT:
		case VK_FORMAT_R8G8_SRGB:
			return 2;
		case VK_FORMAT_R8G8B8_UNORM:
		case VK_FORMAT_R8G8B8_SNORM:
		case VK_FORMAT_R8G8B8_USCALED:
		case VK_FORMAT_R8G8B8_SSCALED:
		case VK_FORMAT_R8G8B8_UINT:
		case VK_FORMAT_R8G8B8_SINT:
		case VK_FORMAT_R8G8B8_SRGB:
		case VK_FORMAT_B8G8R8_UNORM:
		case VK_FORMAT_B8G8R8_SNORM:
		case VK_FORMAT_B8G8R8_USCALED:
		case VK_FORMAT_B8G8R8_SSCALED:
		case VK_FORMAT_B8G8R8_UINT:
		case VK_FORMAT_B8G8R8_SINT:
		case VK_FORMAT_B8G8R8_SRGB:
			return 3;
		case VK_FORMAT_R8G8B8A8_UNORM:
		case VK_FORMAT_R8G8B8A8_SNORM:
		case VK_FORMAT_R8G8B8A8_USCALED:
		case VK_FORMAT_R8G8B8A8_SSCALED:
		case VK_FORMAT_R8G8B8A8_UINT:
		case VK_FORMAT_R8G8B8A8_SINT:
		case VK_FORMAT_R8G8B8A8_SRGB:
		case VK_FORMAT_B8G8R8A8_UNORM:
		case VK_FORMAT_B8G8R8A8_SNORM:
		case VK_FORMAT_B8G8R8A8_USCALED:
		case VK_FORMAT_B8G8R8A8_SSCALED:
		case VK_FORMAT_B8G8R8A8_UINT:
		case VK_FORMAT_B8G8R8A8_SINT:
		case VK_FORMAT_B8G8R8A8_SRGB:
		case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
		case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
		case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
		case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
		case VK_FORMAT_A8B8G8R8_UINT_PACK32:
		case VK_FORMAT_A8B8G8R8_SINT_PACK32:
		case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
		case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
		case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
		case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
		case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
		case VK_FORMAT_A2R10G10B10_UINT_PACK32:
		case VK_FORMAT_A2R10G10B10_SINT_PACK32:
		case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
		case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
		case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
		case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
		case VK_FORMAT_A2B10G10R10_UINT_PACK32:
		case VK_FORMAT_A2B10G10R10_SINT_PACK32:
			return 4;
		case VK_FORMAT_R16_UNORM:
		case VK_FORMAT_R16_SNORM:
		case VK_FORMAT_R16_USCALED:
		case VK_FORMAT_R16_SSCALED:
		case VK_FORMAT_R16_UINT:
		case VK_FORMAT_R16_SINT:
		case VK_FORMAT_R16_SFLOAT:
			return 2;
		case VK_FORMAT_R16G16_UNORM:
		case VK_FORMAT_R16G16_SNORM:
		case VK_FORMAT_R16G16_USCALED:
		case VK_FORMAT_R16G16_SSCALED:
		case VK_FORMAT_R16G16_UINT:
		case VK_FORMAT_R16G16_SINT:
		case VK_FORMAT_R16G16_SFLOAT:
			return 4;
		case VK_FORMAT_R16G16B16_UNORM:
		case VK_FORMAT_R16G16B16_SNORM:
		case VK_FORMAT_R16G16B16_USCALED:
		case VK_FORMAT_R16G16B16_SSCALED:
		case VK_FORMAT_R16G16B16_UINT:
		case VK_FORMAT_R16G16B16_SINT:
		case VK_FORMAT_R16G16B16_SFLOAT:
			return 6;
		case VK_FORMAT_R16G16B16A16_UNORM:
		case VK_FORMAT_R16G16B16A16_SNORM:
		case VK_FORMAT_R16G16B16A16_USCALED:
		case VK_FORMAT_R16G16B16A16_SSCALED:
		case VK_FORMAT_R16G16B16A16_UINT:
		case VK_FORMAT_R16G16B16A16_SINT:
		case VK_FORMAT_R16G16B16A16_SFLOAT:
			return 8;
		case VK_FORMAT_R32_UINT:
		case VK_FORMAT_R32_SINT:
		case VK_FORMAT_R32_SFLOAT:
			return 4;
		case VK_FORMAT_R32G32_UINT:
		case VK_FORMAT_R32G32_SINT:
		case VK_FORMAT_R32G32_SFLOAT:
			return 8;
		case VK_FORMAT_R32G32B32_UINT:
		case VK_FORMAT_R32G32B32_SINT:
		case VK_FORMAT_R32G32B32_SFLOAT:
			return 12;
		case VK_FORMAT_R32G32B32A32_UINT:
		case VK_FORMAT_R32G32B32A32_SINT:
		case VK_FORMAT_R32G32B32A32_SFLOAT:
			return 16;
		case VK_FORMAT_R64_UINT:
		case VK_FORMAT_R64_SINT:
		case VK_FORMAT_R64_SFLOAT:
			return 8;
		case VK_FORMAT_R64G64_UINT:
		case VK_FORMAT_R64G64_SINT:
		case VK_FORMAT_R64G64_SFLOAT:
			return 16;
		case VK_FORMAT_R64G64B64_UINT:
		case VK_FORMAT_R64G64B64_SINT:
		case VK_FORMAT_R64G64B64_SFLOAT:
			return 24;
		case VK_FORMAT_R64G64B64A64_UINT:
		case VK_FORMAT_R64G64B64A64_SINT:
		case VK_FORMAT_R64G64B64A64_SFLOAT:
			return 32;
		case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
		case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
			return 4;

		default:
			break;
		}
		return 0;
	}

	constexpr static bool IsDepthFormat(VkFormat format)
	{
		switch (format)
		{
		case VK_FORMAT_D16_UNORM:
		case VK_FORMAT_X8_D24_UNORM_PACK32:
		case VK_FORMAT_D32_SFLOAT:
		case VK_FORMAT_S8_UINT:
		case VK_FORMAT_D16_UNORM_S8_UINT:
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
			return true;
		default: ;
		}
		return false;
	}

	constexpr static VkPipelineBindPoint GetBindPointForQueueType(QueueType queue_type)
	{
		switch (queue_type)
		{
		case QueueType::Graphics:	return VK_PIPELINE_BIND_POINT_GRAPHICS;
		case QueueType::Compute:	return VK_PIPELINE_BIND_POINT_COMPUTE;
		default:					return VK_PIPELINE_BIND_POINT_GRAPHICS;
		}
	}

	constexpr static VkPrimitiveTopology ConvertToVkPrimitiveTopology(PrimitiveTopology topology)
	{
		switch (topology)
		{
		case PrimitiveTopology::PointLists:					return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case PrimitiveTopology::LineList:					return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case PrimitiveTopology::LineStrip:					return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		case PrimitiveTopology::TriangleList:				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case PrimitiveTopology::TriangleStrip:				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		case PrimitiveTopology::TriangleFan:				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
		case PrimitiveTopology::LineListWithAdjacency:		return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
		case PrimitiveTopology::LineStripWithAdjacency:		return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
		case PrimitiveTopology::TriangleListWithAdjacency:	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
		case PrimitiveTopology::TriangleStripWithAdjacency:	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
		case PrimitiveTopology::PatchList:					return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
		}
	}

	static void PopulateVertexInputBindingDescriptionFromSpirv(SpvReflectShaderModule const& reflect, VkVertexInputBindingDescription& out_vertex_binding, AxArray<VkVertexInputAttributeDescription>& out_vertex_attributes)
	{
		out_vertex_binding = {
			.binding = 0,
			.stride = 0,
		};

		out_vertex_attributes.reserve(reflect.input_variable_count);

		for (u32 i = 0; i < reflect.input_variable_count; i++)
		{
			SpvReflectInterfaceVariable& inputVariable = *reflect.input_variables[i];

			if (inputVariable.decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN)
				continue;

			out_vertex_attributes.append({
				.location = inputVariable.location,
				.binding = out_vertex_binding.binding,
				.format = static_cast<VkFormat>(inputVariable.format),
				.offset = 0,
			});
		}

		std::sort(out_vertex_attributes.begin(), out_vertex_attributes.end(), 
			[](VkVertexInputAttributeDescription const& a, VkVertexInputAttributeDescription const& b)
			{
				return a.location < b.location;
			});

		for (VkVertexInputAttributeDescription& attribute : out_vertex_attributes)
		{
			const u32 formatSize = GetFormatSize(attribute.format);
			attribute.offset = out_vertex_binding.stride;
			out_vertex_binding.stride += formatSize;
		}
	}

	VulkanDevice::VulkanDevice(VulkanContextImpl* context, VkPhysicalDevice physical_device, VkDevice logical_device, VmaAllocator allocator, AxArrayRef<VulkanQueueInfo> queue_infos)
		: m_context(context), m_physicalDevice(physical_device), m_logicalDevice(logical_device), m_allocator(allocator)
	{
		// Store physical device properties
		VkPhysicalDeviceDescriptorIndexingProperties descriptorIndexingProperties { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES };
		m_physicalDeviceProperties.SetupChain(&descriptorIndexingProperties);
		vkGetPhysicalDeviceProperties2(m_physicalDevice, &m_physicalDeviceProperties.properties);

		// Store physical device features
		m_physicalDeviceFeatures.SetupChain();
		vkGetPhysicalDeviceFeatures2(m_physicalDevice, &m_physicalDeviceFeatures.features);

		// Store physical device memory properties
		m_physicalDeviceMemoryProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
		vkGetPhysicalDeviceMemoryProperties2(m_physicalDevice, &m_physicalDeviceMemoryProperties);

		// Store physical device extensions
		u32 extensionCount;
		vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, nullptr);
		m_availableExtensions.resize(extensionCount);
		vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, m_availableExtensions.dataMutable());

		for (u32 i = 0; i < QueueType::COUNT; i++)
		{
			VkQueue queue;
			vkGetDeviceQueue(m_logicalDevice, queue_infos[i].familyIndex, 0, &queue);
			if (queue)
			{
				m_queues[i].m_device = this;
				m_queues[i].m_queue = queue;
				m_queues[i].m_info = queue_infos[i];
				m_queues[i].m_type = i;
			}
		}

		CreateBindlessDescriptorResources();
	}

	VulkanDevice::~VulkanDevice()
	{
		vkDestroySampler(m_logicalDevice, m_defaultNearestSampler, VK_NULL_HANDLE);
	#if GFX_USE_BINDLESS_DESCRIPTORS
		vkDestroyPipelineLayout(m_logicalDevice, m_bindlessPipelineLayout, VK_NULL_HANDLE);
		for (VkDescriptorSetLayout descriptorSetLayout : m_bindlessDescriptorSetLayouts)
		{
			vkDestroyDescriptorSetLayout(m_logicalDevice, descriptorSetLayout, VK_NULL_HANDLE);
		}
	#endif
		vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, VK_NULL_HANDLE);
		DestroyPerFrameData();
		DestroySwapchain();
		for (auto& commandPool : m_commandPools)
		{
			vkDestroyCommandPool(m_logicalDevice, commandPool, VK_NULL_HANDLE);
		}
		vmaDestroyAllocator(m_allocator);
		vkDestroyDevice(m_logicalDevice, VK_NULL_HANDLE);
	}

	CommandBuffer* VulkanDevice::AllocateCommandBuffer(QueueType queue_idx, u32 frame_index, u32 thread_idx) const
	{
		VkCommandBuffer commandBuffer;

		const u32 poolIdx = queue_idx * MAX_FRAMES_IN_FLIGHT * m_renderThreadCount +  frame_index * m_renderThreadCount + thread_idx;

		const VkCommandBufferAllocateInfo allocateInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = m_commandPools[poolIdx],
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};

		axVerifyFmt(VK_SUCCESS == vkAllocateCommandBuffers(m_logicalDevice, &allocateInfo, &commandBuffer),
			"Failed to allocate command buffers!"
		);

		return apex_new VulkanCommandBuffer(this, m_commandPools[poolIdx], commandBuffer, queue_idx, thread_idx);
	}

	const Image* VulkanDevice::AcquireNextImage()
	{
		const VkFence renderFence = GetRenderFence();
		vkWaitForFences(m_logicalDevice, 1, &renderFence, VK_TRUE, 10'000'000);
		vkResetFences(m_logicalDevice, 1, &renderFence);

		const VkAcquireNextImageInfoKHR acquireNextImageInfo {
			.sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
			.swapchain = m_swapchain.handle,
			.timeout = 120'000'000'000 /* ns */, // 120 s = 2 min
			.semaphore = GetImageAcquiredSemaphore(),
			.fence = VK_NULL_HANDLE,
			.deviceMask = 0x1,
		};

		VkResult result = vkAcquireNextImage2KHR(m_logicalDevice, &acquireNextImageInfo, &m_currentSwapchainImageIndex);
		if (VK_ERROR_OUT_OF_DATE_KHR == result)
		{
			GetContext()->ResizeSurface();
		}
		else if (!axVerifyFmt(VK_SUCCESS == result || VK_SUBOPTIMAL_KHR == result, "Failed to acquire swapchain image : {}", string_VkResult(result)))
		{
			return nullptr;
		}

		return &m_swapchainImages[m_currentSwapchainImageIndex];
	}

	void VulkanDevice::WaitForIdle() const
	{
		vkDeviceWaitIdle(m_logicalDevice);
	}

	AxArray<DescriptorSet> VulkanDevice::AllocateDescriptorSets(GraphicsPipeline* pipeline) const
	{
	#if GFX_USE_BINDLESS_DESCRIPTORS
		return {};
	#else
		const VulkanGraphicsPipeline* vkpipeline = static_cast<VulkanGraphicsPipeline*>(pipeline);

		const VkDescriptorSetAllocateInfo allocateInfo {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = m_descriptorPool,
			.descriptorSetCount = static_cast<u32>(vkpipeline->m_descriptorSetLayouts.size()),
			.pSetLayouts = vkpipeline->m_descriptorSetLayouts.data(),
		};

		AxArray<VkDescriptorSet> vkDescriptorSets;
		vkDescriptorSets.resize(vkpipeline->m_descriptorSetLayouts.size());
		vkAllocateDescriptorSets(m_logicalDevice, &allocateInfo, vkDescriptorSets.dataMutable());

		AxArray<DescriptorSet> descriptorSets(vkDescriptorSets.size());
		for (VkDescriptorSet vkDescriptorSet : vkDescriptorSets)
		{
			descriptorSets.emplace_back(vkDescriptorSet);
		}

		return descriptorSets;
	#endif
	}

	void VulkanDevice::UpdateDescriptorSet(DescriptorSet const& descriptor_set) const
	{
		const VkDescriptorSet vkDescriptorSet = static_cast<VkDescriptorSet>(descriptor_set.GetNativeHandle());

		AxArray<VkWriteDescriptorSet> writeDescriptorSets(descriptor_set.GetDescriptors().size());

		for (const Descriptor& descriptor : descriptor_set.GetDescriptors())
		{
			DescriptorType type = descriptor.GetType();

			VkWriteDescriptorSet write {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = vkDescriptorSet,
				.dstBinding = static_cast<u32>(writeDescriptorSets.size()),
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = ConvertToVkDescriptorType(type),
			};

			VkDescriptorImageInfo imageInfo {};
			VkDescriptorBufferInfo bufferInfo {};

			if (descriptor.GetResource()->IsImage())
			{
				axAssertFmt(false, "Not implemented!");
			}
			else
			{
				bufferInfo.buffer = static_cast<VulkanBuffer const*>(descriptor.GetResource())->GetNativeHandle();
				bufferInfo.offset = 0;
				bufferInfo.range = VK_WHOLE_SIZE;

				write.pBufferInfo = &bufferInfo;
			}

			writeDescriptorSets.append(write);
		}

		vkUpdateDescriptorSets(m_logicalDevice, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
	}

	ShaderModule* VulkanDevice::CreateShaderModule(const char* name, const char* filepath) const
	{
		VkShaderModule shader;
		SpvReflectShaderModule reflect;

		AxArray<char> shaderCode;
		{
			const File shaderFile = File::OpenExisting(filepath);
			shaderCode.resize(shaderFile.GetSize());
			shaderFile.Read(shaderCode.dataMutable(), shaderCode.size());
		}

		const VkShaderModuleCreateInfo shaderModuleCreateInfo {
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = shaderCode.size(),
			.pCode = reinterpret_cast<u32*>(shaderCode.data()),
		};

		axVerifyFmt(VK_SUCCESS == vkCreateShaderModule(m_logicalDevice, &shaderModuleCreateInfo, VK_NULL_HANDLE, &shader),
			"Failed to create shader module!"
		);

		spvReflectCreateShaderModule(shaderCode.size(), shaderCode.data(), &reflect);

		SetObjectName(m_logicalDevice, VK_OBJECT_TYPE_SHADER_MODULE, shader, name);

		return apex_new VulkanShaderModule(this, shader, reflect);
	}

	GraphicsPipeline* VulkanDevice::CreateGraphicsPipeline(const char* name, GraphicsPipelineCreateDesc const& desc) const
	{
		AxArray<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;
		shaderStageCreateInfos.resize(2);

		const VulkanShaderModule* vertexShader = static_cast<VulkanShaderModule*>(desc.shaderStages.vertexShader);

		shaderStageCreateInfos[0] = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = vertexShader->m_shader,
			.pName = vertexShader->m_reflect.entry_point_name,
		};

		const VulkanShaderModule* fragmentShader = static_cast<VulkanShaderModule*>(desc.shaderStages.fragmentShader);

		shaderStageCreateInfos[1] = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = fragmentShader->m_shader,
			.pName = fragmentShader->m_reflect.entry_point_name,
		};

		VkVertexInputBindingDescription bindingDescription;
		AxArray<VkVertexInputAttributeDescription> attributeDescriptions;
		PopulateVertexInputBindingDescriptionFromSpirv(vertexShader->m_reflect, bindingDescription, attributeDescriptions);

		const VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.vertexBindingDescriptionCount = 1,
			.pVertexBindingDescriptions = &bindingDescription,
			.vertexAttributeDescriptionCount = static_cast<u32>(attributeDescriptions.size()),
			.pVertexAttributeDescriptions = attributeDescriptions.data(),
		};

		const VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = ConvertToVkPrimitiveTopology(desc.topology),
			.primitiveRestartEnable = VK_FALSE,
		};

		const VkViewport viewport {
			.x = 0.f,
			.y = 0.f,
			.width = static_cast<float>(m_swapchain.extent.width),
			.height = static_cast<float>(m_swapchain.extent.height),
			.minDepth = 0.f,
			.maxDepth = 1.f,
		};

		const VkRect2D scissor {
			.offset = { .x = 0, .y = 0 },
			.extent = m_swapchain.extent,
		};

		const VkPipelineViewportStateCreateInfo viewportStateCreateInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.pViewports = &viewport,
			.scissorCount = 1,
			.pScissors = &scissor,
		};

		const VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_BACK_BIT,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0.f,
			.depthBiasClamp = 0.f,
			.depthBiasSlopeFactor = 0.f,
			.lineWidth = 1.f,
		};

		const VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		};

		const VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = VK_TRUE,
			.depthWriteEnable = VK_TRUE,
			.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
			.front = {},
			.back = {},
			.minDepthBounds = 0.f,
			.maxDepthBounds = 1.f,
		};

		const VkPipelineColorBlendAttachmentState colorBlendAttachment {
			.blendEnable = VK_TRUE,
			.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
			.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			.colorBlendOp = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
			.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
			.alphaBlendOp = VK_BLEND_OP_ADD,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		};

		const VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.attachmentCount = 1,
			.pAttachments = &colorBlendAttachment,
		};

		VkDynamicState dynamicStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
		};

		const VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = std::size(dynamicStates),
			.pDynamicStates = dynamicStates,
		};

	#if GFX_USE_BINDLESS_DESCRIPTORS
		VkPipelineLayout pipelineLayout = m_bindlessPipelineLayout;
	#else
		AxArray<VkDescriptorSetLayout> descriptorSetLayouts;
		descriptorSetLayouts.reserve(1 + vertexModule->m_reflect.descriptor_set_count + fragmentModule->m_reflect.descriptor_set_count);
		for (u32 i = 0; i < vertexModule->m_reflect.descriptor_set_count; i++)
		{
			if (vertexModule->m_reflect.descriptor_sets[i].set == 0)
				continue;

			if (VkDescriptorSetLayout layout = CreateDescriptorSetLayoutFromShader(vertexModule, i))
				descriptorSetLayouts.append(layout);
		}
		for (u32 i = 0; i < fragmentModule->m_reflect.descriptor_set_count; i++)
		{
			if (fragmentModule->m_reflect.descriptor_sets[i].set == 0)
				continue;

			if (VkDescriptorSetLayout layout = CreateDescriptorSetLayoutFromShader(fragmentModule, i))
				descriptorSetLayouts.append(layout);
		}

		axDebugFmt("Pipeline {}: # descriptor sets : {}", name, descriptorSetLayouts.size());

		AxArray<VkPushConstantRange> pushConstantRanges;
		pushConstantRanges.reserve(vertexModule->m_reflect.push_constant_block_count + fragmentModule->m_reflect.push_constant_block_count);
		for (u32 i = 0; i < vertexModule->m_reflect.push_constant_block_count; i++)
		{
			pushConstantRanges.append(CreatePushConstantRangeFromShader(vertexModule, i));
		}
		for (u32 i = 0; i < fragmentModule->m_reflect.push_constant_block_count; i++)
		{
			pushConstantRanges.append(CreatePushConstantRangeFromShader(fragmentModule, i));
		}

		const VkPipelineLayoutCreateInfo layoutCreateInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = static_cast<u32>(descriptorSetLayouts.size()),
			.pSetLayouts = descriptorSetLayouts.data(),
			.pushConstantRangeCount = static_cast<u32>(pushConstantRanges.size()),
			.pPushConstantRanges = pushConstantRanges.data(),
		};

		VkPipelineLayout pipelineLayout;

		axVerifyFmt(VK_SUCCESS == vkCreatePipelineLayout(m_logicalDevice, &layoutCreateInfo, VK_NULL_HANDLE, &pipelineLayout),
			"Failed to create pipeline layout!"
		);
	#endif // GFX_USE_BINDLESS_DESCRIPTORS

		const VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			// TODO: Change this to use the user defined attachments
			.colorAttachmentCount = 1,
			.pColorAttachmentFormats = &m_swapchain.surfaceFormat.format,
			.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT,
		};

		const VkGraphicsPipelineCreateInfo pipelineCreateInfo {
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = &pipelineRenderingCreateInfo,
			.flags = 0,
			.stageCount = static_cast<u32>(shaderStageCreateInfos.size()),
			.pStages = shaderStageCreateInfos.data(),
			.pVertexInputState = &vertexInputStateCreateInfo,
			.pInputAssemblyState = &inputAssemblyStateCreateInfo,
			.pTessellationState = nullptr,
			.pViewportState = &viewportStateCreateInfo,
			.pRasterizationState = &rasterizationStateCreateInfo,
			.pMultisampleState = &multisampleStateCreateInfo,
			.pDepthStencilState = &depthStencilStateCreateInfo,
			.pColorBlendState = &colorBlendStateCreateInfo,
			.pDynamicState = &dynamicStateCreateInfo,
			.layout = pipelineLayout,
			.renderPass = nullptr,
			.subpass = 0,
		};

		VkPipeline pipeline;

		VkResult result = vkCreateGraphicsPipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, VK_NULL_HANDLE, &pipeline);
		axVerifyFmt(VK_SUCCESS == result, "Failed to create graphics pipeline : {}", string_VkResult(result));

		SetObjectName(m_logicalDevice, VK_OBJECT_TYPE_PIPELINE, pipeline, name);

		return apex_new VulkanGraphicsPipeline(this, pipeline, pipelineLayout
	#if GFX_USE_BINDLESS_DESCRIPTORS
	#else
			, descriptorSetLayouts
	#endif
			);
	}

	ComputePipeline* VulkanDevice::CreateComputePipeline(const char* name, ComputePipelineCreateDesc const& desc) const
	{
	#if GFX_USE_BINDLESS_DESCRIPTORS
		VkPipelineLayout pipelineLayout = m_bindlessPipelineLayout;
	#endif

		const VulkanShaderModule* computeShader = static_cast<VulkanShaderModule*>(desc.computeShader);

		const VkComputePipelineCreateInfo pipelineCreateInfo {
			.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.stage = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage = VK_SHADER_STAGE_COMPUTE_BIT,
				.module = computeShader->m_shader,
				.pName = computeShader->m_reflect.entry_point_name,
			},
			.layout = pipelineLayout,
		};

		VkPipeline pipeline;

		VkResult result = vkCreateComputePipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, VK_NULL_HANDLE, &pipeline);
		axVerifyFmt(VK_SUCCESS == result, "Failed to create compute pipeline : {}", string_VkResult(result));

		SetObjectName(m_logicalDevice, VK_OBJECT_TYPE_PIPELINE, pipeline, name);

		return apex_new VulkanComputePipeline(this, pipeline, pipelineLayout
	#if GFX_USE_BINDLESS_DESCRIPTORS
	#else
			, descriptorSetLayouts
	#endif
			);
	}

	Buffer* VulkanDevice::CreateBuffer(const char* name, BufferCreateDesc const& desc)
	{
		u32 queueFamilyIndices[] = { m_queues[desc.ownerQueue].m_info.familyIndex };

		const VkBufferCreateInfo bufferCreateInfo {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = desc.size,
			.usage = ConvertToVkBufferUsageFlags(desc.usageFlags),
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 1,
			.pQueueFamilyIndices = queueFamilyIndices,
		};

		const VmaAllocationCreateInfo allocationCreateInfo {
			.flags = ConvertToVmaAllocationCreateFlags(desc.memoryFlags) | (~(desc.createMapped - 1) & VMA_ALLOCATION_CREATE_MAPPED_BIT),
			.usage = VMA_MEMORY_USAGE_UNKNOWN,
			.requiredFlags = ConvertToVkMemoryPropertyFlags(desc.requiredFlags),
			.preferredFlags = ConvertToVkMemoryPropertyFlags(desc.preferredFlags),
			.memoryTypeBits = 0,
		};

		VkBuffer buffer;
		VmaAllocation allocation;
		VmaAllocationInfo allocationInfo;

		axVerifyFmt(VK_SUCCESS == vmaCreateBuffer(m_allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer, &allocation, &allocationInfo),
			"Failed to create Vulkan Buffer!"
		);

		SetObjectName(m_logicalDevice, VK_OBJECT_TYPE_BUFFER, buffer, name);

		return apex_new VulkanBuffer(this, buffer, bufferCreateInfo, allocation, allocationInfo);
	}

	Buffer* VulkanDevice::CreateVertexBuffer(const char* name, size_t size, const void* initial_data)
	{
		const BufferCreateDesc desc {
			.size = size,
			.usageFlags = BufferUsageFlagBits::Vertex | BufferUsageFlagBits::TransferDst,
			.requiredFlags = MemoryPropertyFlagBits::DeviceLocal,
			.preferredFlags = MemoryPropertyFlagBits::None,
			.memoryFlags = MemoryAllocateFlagBits::None,
			.ownerQueue = QueueType::Graphics,
			.createMapped = false,
			.alignment = 0,
			.pInitialData = initial_data
		};

		return CreateBuffer(name, desc);
	}

	Buffer* VulkanDevice::CreateIndexBuffer(const char* name, size_t size, const void* initial_data)
	{
		const BufferCreateDesc desc {
			.size = size,
			.usageFlags = BufferUsageFlagBits::Index | BufferUsageFlagBits::TransferDst,
			.requiredFlags = MemoryPropertyFlagBits::DeviceLocal,
			.preferredFlags = MemoryPropertyFlagBits::None,
			.memoryFlags = MemoryAllocateFlagBits::None,
			.ownerQueue = QueueType::Graphics,
			.createMapped = false,
			.alignment = 0,
			.pInitialData = initial_data
		};

		return CreateBuffer(name, desc);
	}

	Buffer* VulkanDevice::CreateStagingBuffer(const char* name, size_t size)
	{
		const BufferCreateDesc desc {
			.size = size,
			.usageFlags = BufferUsageFlagBits::TransferSrc,
			.requiredFlags = MemoryPropertyFlagBits::HostCoherent,
			.preferredFlags = MemoryPropertyFlagBits::DeviceLocal,
			.memoryFlags = MemoryAllocateFlagBits::HostAccessSequential,
			.ownerQueue = QueueType::Graphics,
			.createMapped = true,
		};

		return CreateBuffer(name, desc);
	}

	Image* VulkanDevice::CreateImage(const char* name, ImageCreateDesc const& desc)
	{
		u32 queueFamilyIndices[] = { m_queues[QueueType::Graphics].m_info.familyIndex };

		const VkImageCreateInfo imageCreateInfo {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.flags = 0, // CUBE_COMPATIBLE_BIT for cubemap
		    .imageType = ConvertToVkImageType(desc.imageType),
		    .format = ConvertToVkFormat(desc.format),
			.extent = ConvertToVkExtent3D(desc.dimensions),
		    .mipLevels = 1,
		    .arrayLayers = 1,
		    .samples = VK_SAMPLE_COUNT_1_BIT,
		    .tiling = VK_IMAGE_TILING_OPTIMAL,
		    .usage = ConvertToVkImageUsageFlags(desc.usageFlags),
		    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		    .queueFamilyIndexCount = 1,
		    .pQueueFamilyIndices = queueFamilyIndices,
		    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		};

		const VmaAllocationCreateInfo allocationCreateInfo {
			.flags = ConvertToVmaAllocationCreateFlags(desc.memoryFlags) | (~(desc.createMapped - 1) & VMA_ALLOCATION_CREATE_MAPPED_BIT),
			.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
			.requiredFlags = ConvertToVkMemoryPropertyFlags(desc.requiredFlags),
			.preferredFlags = ConvertToVkMemoryPropertyFlags(desc.preferredFlags),
			.memoryTypeBits = 0,
		};

		VkImage image;
		VkImageView imageView;
		VmaAllocation allocation;
		VmaAllocationInfo allocationInfo;

		axVerifyFmt(VK_SUCCESS == vmaCreateImage(m_allocator, &imageCreateInfo, &allocationCreateInfo, &image, &allocation, &allocationInfo),
			"Failed to create Vulkan Image!"
		);

		SetObjectName(m_logicalDevice, VK_OBJECT_TYPE_IMAGE, image, name);

		const VkImageViewCreateInfo imageViewCreateInfo {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = image,
		    .viewType = ConvertToVkImageViewType(desc.imageType),
		    .format = ConvertToVkFormat(desc.format),
		    .components = {
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY,
			},
			.subresourceRange = {
				.aspectMask = (VkImageAspectFlags)(desc.format >= ImageFormat::D16_UNORM ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT),
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};

		axVerifyFmt(VK_SUCCESS == vkCreateImageView(m_logicalDevice, &imageViewCreateInfo, nullptr, &imageView),
			"Failed to create Vulkan Image View!"
		);

		SetObjectName(m_logicalDevice, VK_OBJECT_TYPE_IMAGE_VIEW, imageView, name);
		
		return apex_new VulkanImage(this, image, imageCreateInfo, allocation, allocationInfo, imageView);
	}

	ImageView* VulkanDevice::CreateImageView(const char* name, Image const* image) const
	{
		return nullptr;
	}

	ImageView* VulkanDevice::CreateImageView(const char* name, ImageViewCreateDesc const& desc) const
	{
		return nullptr;
	}

	Fence* VulkanDevice::CreateFence(const char* name, u64 init_value)
	{
		const VkSemaphoreTypeCreateInfo semaphoreTypeCreateInfo {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
			.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
			.initialValue = init_value
		};

		const VkSemaphoreCreateInfo semaphoreCreateInfo {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = &semaphoreTypeCreateInfo,
			.flags = 0,
		};

		VkSemaphore timelineSemaphore;

		axVerifyFmt(VK_SUCCESS == vkCreateSemaphore(m_logicalDevice, &semaphoreCreateInfo, VK_NULL_HANDLE, &timelineSemaphore),
			"Failed to create Vulkan timeline semaphore!"
		);

		SetObjectName(m_logicalDevice, VK_OBJECT_TYPE_SEMAPHORE, timelineSemaphore, name);

		return apex_new VulkanFence(this, timelineSemaphore, init_value);
	}

	void VulkanDevice::BindSampledImage(ImageView* image_view)
	{
		VulkanImageView* vkImageView = static_cast<VulkanImageView*>(image_view);

		if (!axVerifyFmt(vkImageView->m_bindlessIndices[0] == (u32)-1, "Image view is already bound as Sampled Image {}", vkImageView->m_bindlessIndices[0]))
			return;

		axAssert(vkImageView->m_owner->GetUsageFlags() & VK_IMAGE_USAGE_SAMPLED_BIT);

		const u32 imageIndex = m_nextSampledImageBindingIndex.fetch_add(1);

		const VkDescriptorImageInfo imageInfo { 
			.sampler = m_defaultNearestSampler,
			.imageView = vkImageView->GetNativeHandle(),
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		const VkWriteDescriptorSet writes[] = {
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = m_bindlessDescriptorSets[BindlessDescriptorType::SampledImage],
				.dstBinding = 0,
				.dstArrayElement = imageIndex,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
				.pImageInfo = &imageInfo,
			}
		};

		vkUpdateDescriptorSets(m_logicalDevice, 1, writes, 0, nullptr); // TODO: Buffer multiple descriptor writes and update at once

		vkImageView->m_bindlessIndices[0] = imageIndex;
	}

	void VulkanDevice::BindStorageImage(ImageView* image_view)
	{
		VulkanImageView* vkImageView = static_cast<VulkanImageView*>(image_view);

		if (!axVerifyFmt(vkImageView->m_bindlessIndices[1] == (u32)-1, "Image view is already bound as Storage Image {}", vkImageView->m_bindlessIndices[1]))
			return;

		axAssert(vkImageView->m_owner->GetUsageFlags() & VK_IMAGE_USAGE_STORAGE_BIT);

		const u32 imageIndex = m_nextStorageImageBindingIndex.fetch_add(1);

		const VkDescriptorImageInfo imageInfo { 
			.sampler = m_defaultNearestSampler,
			.imageView = vkImageView->GetNativeHandle(),
			.imageLayout = VK_IMAGE_LAYOUT_GENERAL
		};

		const VkWriteDescriptorSet writes[] = {
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = m_bindlessDescriptorSets[BindlessDescriptorType::StorageImage],
				.dstBinding = 0,
				.dstArrayElement = imageIndex,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				.pImageInfo = &imageInfo,
			}
		};

		vkUpdateDescriptorSets(m_logicalDevice, 1, writes, 0, nullptr); // TODO: Buffer multiple descriptor writes and update at once

		vkImageView->m_bindlessIndices[1] = imageIndex;
	}

	void VulkanDevice::BindUniformBuffer(Buffer* buffer)
	{
		VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);

		if (!axVerifyFmt(vkBuffer->m_bindlessIndices[0] == (u32)-1, "Buffer is already bound as Uniform Buffer {}", vkBuffer->m_bindlessIndices[0]))
			return;

		axAssert(vkBuffer->GetUsageFlags() & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		const u32 bufferIndex = m_nextUniformBufferBindingIndex.fetch_add(1);

		const VkDescriptorBufferInfo bufferInfo {
			.buffer = vkBuffer->GetNativeHandle(),
			.offset = 0,
			.range = VK_WHOLE_SIZE,
		};

		const VkWriteDescriptorSet writes[] = {
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = m_bindlessDescriptorSets[BindlessDescriptorType::UniformBuffer],
				.dstBinding = 0,
				.dstArrayElement = bufferIndex,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.pBufferInfo = &bufferInfo,
			}
		};

		vkUpdateDescriptorSets(m_logicalDevice, 1, writes, 0, nullptr); // TODO: Buffer multiple descriptor writes and update at once

		vkBuffer->m_bindlessIndices[0] = bufferIndex;
	}

	void VulkanDevice::BindStorageBuffer(Buffer* buffer)
	{
		VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);

		if (!axVerifyFmt(vkBuffer->m_bindlessIndices[1] == (u32)-1, "Buffer is already bound as Storage Buffer {}", vkBuffer->m_bindlessIndices[1]))
			return;

		axAssert(vkBuffer->GetUsageFlags() & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

		const u32 bufferIndex = m_nextStorageBufferBindingIndex.fetch_add(1);

		const VkDescriptorBufferInfo bufferInfo {
			.buffer = vkBuffer->GetNativeHandle(),
			.offset = 0,
			.range = VK_WHOLE_SIZE,
		};

		const VkWriteDescriptorSet writes[] = {
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = m_bindlessDescriptorSets[BindlessDescriptorType::StorageBuffer],
				.dstBinding = 0,
				.dstArrayElement = bufferIndex,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				.pBufferInfo = &bufferInfo,
			}
		};

		vkUpdateDescriptorSets(m_logicalDevice, 1, writes, 0, nullptr); // TODO: Buffer multiple descriptor writes and update at once

		vkBuffer->m_bindlessIndices[1] = bufferIndex;
	}

	void VulkanDevice::DestroyShaderModule(ShaderModule* shader) const
	{
		VulkanShaderModule* vkshader = static_cast<VulkanShaderModule*>(shader);
		spvReflectDestroyShaderModule(&vkshader->m_reflect);
		vkDestroyShaderModule(m_logicalDevice, vkshader->m_shader, VK_NULL_HANDLE);
	}

	void VulkanDevice::DestroyPipeline(GraphicsPipeline* pipeline) const
	{
		VulkanGraphicsPipeline* vkpipeline = static_cast<VulkanGraphicsPipeline*>(pipeline);
	#if GFX_USE_BINDLESS_DESCRIPTORS
	#else
		for (VkDescriptorSetLayout& descriptorSetLayout : vkpipeline->m_descriptorSetLayouts)
		{
			vkDestroyDescriptorSetLayout(m_logicalDevice, descriptorSetLayout, VK_NULL_HANDLE);
		}
		vkDestroyPipelineLayout(m_logicalDevice, vkpipeline->m_pipelineLayout, VK_NULL_HANDLE);
	#endif
		vkDestroyPipeline(m_logicalDevice, vkpipeline->m_pipeline, VK_NULL_HANDLE);
	}

	void VulkanDevice::DestroyPipeline(ComputePipeline* pipeline) const
	{
		VulkanComputePipeline* vkpipeline = static_cast<VulkanComputePipeline*>(pipeline);
	#if GFX_USE_BINDLESS_DESCRIPTORS
	#else
		vkDestroyPipelineLayout(m_logicalDevice, vkpipeline->m_pipelineLayout, VK_NULL_HANDLE);
	#endif
		vkDestroyPipeline(m_logicalDevice, vkpipeline->m_pipeline, VK_NULL_HANDLE);
	}

	void VulkanDevice::DestroyBuffer(Buffer* buffer) const
	{
		VulkanBuffer* vkbuffer = static_cast<VulkanBuffer*>(buffer);
		vmaDestroyBuffer(m_allocator, vkbuffer->m_buffer, vkbuffer->m_allocation);
	}

	void VulkanDevice::DestroyImage(Image* image) const
	{
		VulkanImage* vkimage = static_cast<VulkanImage*>(image);
		if (vkimage->m_allocation)
			vmaDestroyImage(m_allocator, vkimage->m_image, vkimage->m_allocation);
	}

	void VulkanDevice::DestroyImageView(ImageView* view) const
	{
		VulkanImageView* vkview = static_cast<VulkanImageView*>(view);
		vkDestroyImageView(m_logicalDevice, vkview->m_view, VK_NULL_HANDLE);
	}

	void VulkanDevice::DestroyFence(Fence* fence) const
	{
		VulkanFence* vkfence = static_cast<VulkanFence*>(fence);
		vkDestroySemaphore(m_logicalDevice, vkfence->m_semaphore, VK_NULL_HANDLE);
	}

	void VulkanDevice::CreateSwapchain(VkSurfaceKHR surface, u32 width, u32 height)
	{
		VulkanSwapchainSupportDetails swapchainSupportDetails;
		QuerySwapchainSupportDetails(m_physicalDevice, surface, swapchainSupportDetails);

		// Choose surface format
		m_swapchain.surfaceFormat = swapchainSupportDetails.formats[0]; // be default choose the first format
		constexpr static VkSurfaceFormatKHR kPreferredFormat { .format = VK_FORMAT_B8G8R8A8_SRGB, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		for (const VkSurfaceFormatKHR& format : swapchainSupportDetails.formats)
		{
			if (kPreferredFormat.format == format.format && kPreferredFormat.colorSpace == format.colorSpace)
			{
				m_swapchain.surfaceFormat = format;
				break;
			}
		}

		// Choose present mode
		m_swapchain.presentMode = VK_PRESENT_MODE_FIFO_KHR; // by default select FIFO, since it is guaranteed to be available by the spec
		for (const VkPresentModeKHR presentMode : swapchainSupportDetails.presentModes)
		{
			if (VK_PRESENT_MODE_MAILBOX_KHR == presentMode)
			{
				m_swapchain.presentMode = presentMode;
				break;
			}
		}

		// Choose extent
		if (Constants::u32_MAX != swapchainSupportDetails.capabilities.currentExtent.width)
		{
			m_swapchain.extent = swapchainSupportDetails.capabilities.currentExtent;
		}
		else
		{
			// Find the best set of extents to match the window width and height
			VkExtent2D actualExtent { width, height };
			actualExtent.width = std::clamp(actualExtent.width, swapchainSupportDetails.capabilities.minImageExtent.width, swapchainSupportDetails.capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, swapchainSupportDetails.capabilities.minImageExtent.height, swapchainSupportDetails.capabilities.maxImageExtent.height);
			m_swapchain.extent = actualExtent;
		}

		// Choose desired image count
		u32 imageCount = std::max((u32)MAX_FRAMES_IN_FLIGHT, swapchainSupportDetails.capabilities.minImageCount + 1);
		if (swapchainSupportDetails.capabilities.maxImageCount > 0)
		{
			imageCount = std::min(imageCount, swapchainSupportDetails.capabilities.maxImageCount);
		}
		axAssertFmt(imageCount > 0, "Vulkan swapchain must support at least one image!");

		VkSwapchainKHR newSwapchain;

		const VkSwapchainCreateInfoKHR swapchainCreateInfo {
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = surface,

			// Image options
			.minImageCount = imageCount,
			.imageFormat = m_swapchain.surfaceFormat.format,
			.imageColorSpace = m_swapchain.surfaceFormat.colorSpace,
			.imageExtent = m_swapchain.extent,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = nullptr,

			.preTransform = swapchainSupportDetails.capabilities.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,

			.presentMode = m_swapchain.presentMode,
			.clipped = VK_TRUE,
			.oldSwapchain = m_swapchain.handle,
		};

		axVerifyFmt(VK_SUCCESS == vkCreateSwapchainKHR(m_logicalDevice, &swapchainCreateInfo, VK_NULL_HANDLE, &newSwapchain),
			"Failed to create Vulkan swapchain!"
		);

		// Destroy old swapchain and per frame data
		if (VK_NULL_HANDLE != m_swapchain.handle)
		{
			DestroyPerFrameData();
			DestroySwapchain();
		}

		m_swapchain.handle = newSwapchain;

		// Store swapchain images
		AxArray<VkImage> images;
		vkGetSwapchainImagesKHR(m_logicalDevice, m_swapchain.handle, &imageCount, nullptr);
		images.resize(imageCount);
		vkGetSwapchainImagesKHR(m_logicalDevice, m_swapchain.handle, &imageCount, images.data());

		// Create image views
		m_swapchainImages.reserve(imageCount);

		for (u32 i = 0; i < imageCount; i++)
		{
			VkImageViewCreateInfo imageViewCreateInfo {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = images[i],
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = m_swapchain.surfaceFormat.format,
				.components = {
					.r = VK_COMPONENT_SWIZZLE_IDENTITY,
					.g = VK_COMPONENT_SWIZZLE_IDENTITY,
					.b = VK_COMPONENT_SWIZZLE_IDENTITY,
					.a = VK_COMPONENT_SWIZZLE_IDENTITY,
				},
				.subresourceRange = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
				},
			};

			VkImageView imageView;

			axVerifyFmt(VK_SUCCESS == vkCreateImageView(m_logicalDevice, &imageViewCreateInfo, VK_NULL_HANDLE, &imageView),
				"Failed to create swapchain image view!"
			);

			m_swapchainImages.emplace_back(this, images[i], imageView, VkExtent3D{ m_swapchain.extent.width, m_swapchain.extent.height, 1 }, m_swapchain.surfaceFormat.format, 0);
		}

		m_swapchainImageCount = imageCount;

		CreatePerFrameData();
	}

	void VulkanDevice::DestroySwapchain()
	{
		m_swapchainImages.reset();
		vkDestroySwapchainKHR(m_logicalDevice, m_swapchain.handle, VK_NULL_HANDLE);
	}

	void VulkanDevice::CreateBindlessDescriptorResources()
	{
		const VkPhysicalDeviceDescriptorIndexingProperties* descriptorIndexingProperties = 
			static_cast<const VkPhysicalDeviceDescriptorIndexingProperties*>(FindInStructureChain(
				VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES,
				&m_physicalDeviceProperties.properties));

		axAssertFmt(descriptorIndexingProperties, "VkPhysicalDeviceDescriptorIndexingProperties not found in VkPhysicalDeviceProperties!");

		const u32 totalSampledImageDescriptors = std::min(2048u, descriptorIndexingProperties->maxDescriptorSetUpdateAfterBindSampledImages);
		const u32 totalUniformBufferDescriptors = std::min(2048u, descriptorIndexingProperties->maxDescriptorSetUpdateAfterBindUniformBuffers);
		const u32 totalStorageImageDescriptors = std::min(2048u, descriptorIndexingProperties->maxDescriptorSetUpdateAfterBindStorageImages);
		const u32 totalStorageBufferDescriptors = std::min(2048u, descriptorIndexingProperties->maxDescriptorSetUpdateAfterBindStorageBuffers);

		axAssert(totalSampledImageDescriptors > 16);
		axAssert(totalStorageImageDescriptors > 16);
		axAssert(totalUniformBufferDescriptors > 16);
		axAssert(totalStorageBufferDescriptors > 16);

		axDebugFmt("Total Sampled Image Descriptors:  {} (Max: {})", totalSampledImageDescriptors, descriptorIndexingProperties->maxDescriptorSetUpdateAfterBindSampledImages);
		axDebugFmt("Total Storage Image Descriptors:  {} (Max: {})", totalStorageImageDescriptors, descriptorIndexingProperties->maxDescriptorSetUpdateAfterBindUniformBuffers);
		axDebugFmt("Total Uniform Buffer Descriptors: {} (Max: {})", totalUniformBufferDescriptors, descriptorIndexingProperties->maxDescriptorSetUpdateAfterBindStorageImages);
		axDebugFmt("Total Storage Buffer Descriptors: {} (Max: {})", totalStorageBufferDescriptors, descriptorIndexingProperties->maxDescriptorSetUpdateAfterBindStorageBuffers);

		const VkDescriptorPoolSize poolSizes[] = {
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, totalSampledImageDescriptors },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, totalStorageImageDescriptors },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, totalUniformBufferDescriptors },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, totalStorageBufferDescriptors },
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 4 }, // TODO: Collect immutable samplers
		};

		const VkDescriptorPoolCreateInfo descriptorPoolCreateInfo {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT, //VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
			.maxSets = 16,
			.poolSizeCount = std::size(poolSizes),
			.pPoolSizes = poolSizes,
		};

		axVerifyFmt(VK_SUCCESS == vkCreateDescriptorPool(m_logicalDevice, &descriptorPoolCreateInfo, VK_NULL_HANDLE, &m_descriptorPool),
			"Failed to create descriptor pool!"
		);

		{
			const VkDescriptorBindingFlags flags {
				VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
					| VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT
					| VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
			};

			const VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo {
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
				.bindingCount = 1,
				.pBindingFlags = &flags,
			};

			for (u32 i = 0; i < 4; i++)
			{
				const VkDescriptorSetLayoutBinding binding {
					.binding = 0,
					.descriptorType = poolSizes[i].type,
					.descriptorCount = poolSizes[i].descriptorCount,
					.stageFlags = VK_SHADER_STAGE_ALL,
				};

				const VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo {
					.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
					.pNext = &bindingFlagsCreateInfo,
					.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
					.bindingCount = 1,
					.pBindings = &binding,
				};

				axVerifyFmt(VK_SUCCESS == vkCreateDescriptorSetLayout(m_logicalDevice, &descriptorSetLayoutCreateInfo, VK_NULL_HANDLE, &m_bindlessDescriptorSetLayouts[i]),
					"Failed to create bindless descriptor set layout!"
				);
			}

			// Create Immutable samplers
			{
				const VkSamplerCreateInfo samplerCreateInfo {
					.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
					.magFilter = VK_FILTER_NEAREST,
					.minFilter = VK_FILTER_NEAREST,
				};

				axVerifyFmt(VK_SUCCESS == vkCreateSampler(m_logicalDevice, &samplerCreateInfo, VK_NULL_HANDLE, &m_defaultNearestSampler),
					"Failed to create Default Nearest Sampler!"
				);

				const VkSampler immutableSamplers[] = {
					m_defaultNearestSampler,
					//m_defaultLinearSampler,
					//m_defaultBilinearSampler,
				};

				const VkDescriptorSetLayoutBinding binding {
					.binding = 0,
					.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
					.descriptorCount = std::size(immutableSamplers),
					.stageFlags = VK_SHADER_STAGE_ALL,
					.pImmutableSamplers = immutableSamplers
				};

				const VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo {
					.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.bindingCount = 1,
					.pBindings = &binding,
				};

				axVerifyFmt(VK_SUCCESS == vkCreateDescriptorSetLayout(m_logicalDevice, &descriptorSetLayoutCreateInfo, VK_NULL_HANDLE, &m_bindlessDescriptorSetLayouts[BindlessDescriptorType::Sampler]),
					"Failed to create bindless descriptor set layout!"
				);
			}

			const VkPushConstantRange pushConstantRanges[] = {
				{
					.stageFlags = VK_SHADER_STAGE_ALL,
					.offset = 0,
					.size = 128,
				}
			};

			const VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
				.setLayoutCount = static_cast<u32>(m_bindlessDescriptorSetLayouts.size()),
				.pSetLayouts = m_bindlessDescriptorSetLayouts.data(),
				.pushConstantRangeCount = static_cast<u32>(std::size(pushConstantRanges)),
				.pPushConstantRanges = pushConstantRanges, // TODO: Add push constant ranges
			};

			axVerifyFmt(VK_SUCCESS == vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutCreateInfo, VK_NULL_HANDLE, &m_bindlessPipelineLayout),
				"Failed to create a pipeline layout!"
			);

			const u32 descriptorSetDescriptorCounts[] = {
				poolSizes[0].descriptorCount,
				poolSizes[1].descriptorCount,
				poolSizes[2].descriptorCount,
				poolSizes[3].descriptorCount,
			};

			const VkDescriptorSetVariableDescriptorCountAllocateInfo variableDescriptorCountAllocateInfo {
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
				.descriptorSetCount = static_cast<u32>(m_bindlessDescriptorSetLayouts.size()) - 1,
				.pDescriptorCounts = descriptorSetDescriptorCounts,
			};

			const VkDescriptorSetAllocateInfo descriptorSetAllocateInfo {
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
				.pNext = &variableDescriptorCountAllocateInfo,
				.descriptorPool = m_descriptorPool,
				.descriptorSetCount = static_cast<u32>(m_bindlessDescriptorSetLayouts.size()) - 1,
				.pSetLayouts = m_bindlessDescriptorSetLayouts.data(),
			};

			axVerifyFmt(VK_SUCCESS == vkAllocateDescriptorSets(m_logicalDevice, &descriptorSetAllocateInfo, m_bindlessDescriptorSets.data()),
				"Failed to allocate bindless descriptor set!"
			);

			{
				const VkDescriptorSetAllocateInfo samplerDescriptorSetAllocateInfo {
					.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
					.pNext = nullptr,
					.descriptorPool = m_descriptorPool,
					.descriptorSetCount = 1,
					.pSetLayouts = &m_bindlessDescriptorSetLayouts[BindlessDescriptorType::Sampler],
				};

				axVerifyFmt(VK_SUCCESS == vkAllocateDescriptorSets(m_logicalDevice, &samplerDescriptorSetAllocateInfo, &m_bindlessDescriptorSets[BindlessDescriptorType::Sampler]),
					"Failed to allocate bindless sampler descriptor set!"
				);
			}
		}
	}

	void VulkanDevice::CreatePerFrameData()
	{
		const VkSemaphoreCreateInfo semaphoreCreateInfo { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

		// m_recycledSemaphores.reserve(MAX_FRAMES_IN_FLIGHT + 1);
		for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkSemaphore semaphore {};
			axVerifyFmt(VK_SUCCESS == vkCreateSemaphore(m_logicalDevice, &semaphoreCreateInfo, VK_NULL_HANDLE, &semaphore),
				"Failed to create semaphore!"
			);
			m_imageAcquiredSemaphores[i] = semaphore;
			// m_recycledSemaphores.append(semaphore);
		}

		for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkSemaphore semaphore {};
			axVerifyFmt(VK_SUCCESS == vkCreateSemaphore(m_logicalDevice, &semaphoreCreateInfo, VK_NULL_HANDLE, &semaphore),
				"Failed to create semaphore!"
			);
			m_renderCompleteSemaphores[i] = semaphore;
		}

		const VkFenceCreateInfo fenceCreateInfo {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT,
		};

		for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkFence fence {};
			axVerifyFmt(VK_SUCCESS == vkCreateFence(m_logicalDevice, &fenceCreateInfo, VK_NULL_HANDLE, &fence),
				"Failed to create fence!"
			);
			m_renderFences[i] = fence;
		}
	}

	void VulkanDevice::DestroyPerFrameData()
	{
		/*for (VkSemaphore& semaphore : m_recycledSemaphores)
		{
			vkDestroySemaphore(m_logicalDevice, semaphore, VK_NULL_HANDLE);
		}*/

		for (VkSemaphore& semaphore : m_imageAcquiredSemaphores)
		{
			vkDestroySemaphore(m_logicalDevice, semaphore, VK_NULL_HANDLE);
		}

		for (VkSemaphore& semaphore : m_renderCompleteSemaphores)
		{
			vkDestroySemaphore(m_logicalDevice, semaphore, VK_NULL_HANDLE);
		}

		for (VkFence& fence : m_renderFences)
		{
			vkDestroyFence(m_logicalDevice, fence, VK_NULL_HANDLE);
		}

		/*m_recycledSemaphores.clear();
		m_imageAcquiredSemaphores.clear();
		m_renderCompleteSemaphores.clear();
		m_renderFences.clear();*/
	}

	void VulkanDevice::CreateCommandPools()
    {
		// Create a command pool for each frame in flight and each render thread
		for (u32 queueIdx = 0; queueIdx < 3; queueIdx++)
		{
	        for (u32 frameIdx = 0; frameIdx < m_swapchainImageCount; frameIdx++)
	        {
	            for (u32 threadIdx = 0; threadIdx < m_renderThreadCount; threadIdx++)
	            {
	                VkCommandPoolCreateInfo poolCreateInfo{
						.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
						.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
						.queueFamilyIndex = m_queues[queueIdx].m_info.familyIndex,
	                };

	                VkCommandPool commandPool;
	                axVerifyFmt(VK_SUCCESS == vkCreateCommandPool(m_logicalDevice, &poolCreateInfo, nullptr, &commandPool),
						"Failed to create command pool!"
					);

					const u32 poolIdx = CalculateCommandPoolIndex(queueIdx, frameIdx, threadIdx);
	                m_commandPools[poolIdx] = commandPool;
	            }
	        }
		}
    }

    VkDescriptorSetLayout VulkanDevice::CreateDescriptorSetLayoutFromShader(VulkanShaderModule const* shader_module, u32 set_idx) const
    {
		const SpvReflectShaderModule& reflect = shader_module->m_reflect;

		const SpvReflectDescriptorSet& reflectSet = reflect.descriptor_sets[set_idx];

		AxArray<VkDescriptorSetLayoutBinding> layoutBindings;
		layoutBindings.resize(reflectSet.binding_count);

		axDebugFmt("Shader descriptor set : {} [{}]", reflect.source_file ? reflect.source_file : string_VkShaderStageFlagBits(static_cast<VkShaderStageFlagBits>(reflect.shader_stage)), reflectSet.set);

		for (u32 bindingIdx = 0; bindingIdx < reflectSet.binding_count; bindingIdx++)
		{
			const SpvReflectDescriptorBinding& reflectBinding = *reflectSet.bindings[bindingIdx];
			if (std::ranges::find_if(layoutBindings,
			                         [binding=reflectBinding.binding](VkDescriptorSetLayoutBinding const& layoutBinding) -> bool
			                         {
				                         return layoutBinding.binding == binding;
			                         })
				!= layoutBindings.end())
			{
				continue;
			}

			VkDescriptorSetLayoutBinding& layoutBinding = layoutBindings[bindingIdx];

			layoutBinding = {
				.binding = reflectBinding.binding,
				.descriptorType = static_cast<VkDescriptorType>(reflectBinding.descriptor_type),
				.descriptorCount = 1,
				.stageFlags = static_cast<VkShaderStageFlags>(reflect.shader_stage),
			};

			axDebugFmt("    [{}] Binding #{}: {}", bindingIdx, layoutBinding.binding, string_VkDescriptorType(layoutBinding.descriptorType));

			for (u32 dimIdx = 0; dimIdx < reflectBinding.array.dims_count; dimIdx++)
			{
				layoutBinding.descriptorCount *= reflectBinding.array.dims[dimIdx];
			}
		}

		const VkDescriptorSetLayoutCreateInfo layoutCreateInfo {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = reflectSet.binding_count,
			.pBindings = layoutBindings.data(),
		};

		VkDescriptorSetLayout layout;

		axVerifyFmt(VK_SUCCESS == vkCreateDescriptorSetLayout(m_logicalDevice, &layoutCreateInfo, VK_NULL_HANDLE, &layout),
			"Failed to create descriptor set!"
		);

		return layout;
	}

    VkPushConstantRange VulkanDevice::CreatePushConstantRangeFromShader(VulkanShaderModule const* shader_module, u32 push_constant_idx) const
    {
		const SpvReflectShaderModule& reflect = shader_module->m_reflect;

		const SpvReflectBlockVariable& reflectPushConstantBlock = reflect.push_constant_blocks[push_constant_idx];

		VkPushConstantRange pushConstantRange {
			.stageFlags = static_cast<VkShaderStageFlags>(reflect.shader_stage),
			.offset = reflectPushConstantBlock.offset,
			.size = reflectPushConstantBlock.size,
		};

		return pushConstantRange;
    }

	// Vulkan Context
    void VulkanContext::Init(const plat::PlatformWindow& window)
    {
		m_pImpl = apex_new VulkanContextImpl((HINSTANCE)window.GetOsApplicationHandle(), (HWND)window.GetOsHandle());
    }

	void VulkanContext::Shutdown()
	{
		delete m_pImpl;
	}

	Device* VulkanContext::GetDevice() const
	{
		return m_pImpl->m_device.get();
	}

	void VulkanContext::GetDeviceFeatures(DeviceFeatures& device_features) const
	{
	}

	void VulkanContext::GetDeviceProperties(DeviceProperties& device_properties) const
	{
	}

	void VulkanContext::ResizeSurface(u32 width, u32 height) const
	{
		(void)m_pImpl->ResizeSurface(width, height);
	}

	void VulkanContext::ResizeSurface() const
	{
		m_pImpl->ResizeSurface();
	}

	VkInstance VulkanContext::GetInstance() const
	{
		return m_pImpl->m_instance;
	}

	// Vulkan Queue
	void VulkanQueue::ResetCommandBuffers(u32 frame_idx, u32 thread_idx) const
	{
		const u32 poolIdx = m_device->CalculateCommandPoolIndex(m_type, frame_idx, thread_idx);
		vkResetCommandPool(m_device->GetLogicalDevice(), m_device->GetCommandPool(poolIdx), 0);
	}

	void VulkanQueue::Submit(CommandBuffer* command_buffer)
	{
		const VkCommandBufferSubmitInfo commandBufferSubmitInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
			.commandBuffer = static_cast<VkCommandBuffer>(command_buffer->GetNativeHandle()),
			.deviceMask = 0,
		};

		const VkSubmitInfo2 submitInfo {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.waitSemaphoreInfoCount = 0,
			.pWaitSemaphoreInfos = nullptr,
			.commandBufferInfoCount = 1,
			.pCommandBufferInfos = &commandBufferSubmitInfo,
			.signalSemaphoreInfoCount = 0,
			.pSignalSemaphoreInfos = nullptr,
		};

		axVerifyFmt(VK_SUCCESS == vkQueueSubmit2(m_queue, 1, &submitInfo, nullptr),
			"Failed to submit command buffer!"
		);
	}

	void VulkanQueue::Submit(CommandBuffer* command_buffer, bool wait_image_acquired, PipelineStageFlags wait_stage_mask, bool signal_render_complete)
	{
		VkCommandBuffer			commandBuffer []		= { static_cast<VkCommandBuffer>(command_buffer->GetNativeHandle()) };

		VkSemaphore				waitSemaphores[]		= { m_device->GetImageAcquiredSemaphore() };
		VkPipelineStageFlags	waitStageMasks[]		= { ConvertToVkPipelineStageFlags(wait_stage_mask) };
		u32						waitCount				= wait_image_acquired ? 1 : 0;

		VkSemaphore				signalSemaphores[]		= { m_device->GetRenderCompleteSemaphore() };
		u32						signalCount				= signal_render_complete ? 1 : 0;

		const VkSubmitInfo submitInfo {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = waitCount,
			.pWaitSemaphores = waitSemaphores,
			.pWaitDstStageMask = waitStageMasks,
			.commandBufferCount = 1,
			.pCommandBuffers = commandBuffer,
			.signalSemaphoreCount = signalCount,
			.pSignalSemaphores = signalSemaphores
		};

		axVerifyFmt(VK_SUCCESS == vkQueueSubmit(m_queue, 1, &submitInfo, m_device->GetRenderFence()),
			"Failed to submit command buffer!"
		);
	}

	void VulkanQueue::Submit(CommandBuffer* command_buffer, Fence* fence, u64 wait_value, PipelineStageFlags wait_stage_mask, u64 signal_value)
	{
		VkCommandBuffer			commandBuffer []		= { static_cast<VkCommandBuffer>(command_buffer->GetNativeHandle()) };

		VkSemaphore				waitSemaphores[]		= { static_cast<VulkanFence*>(fence)->GetSemaphore() };
		u64						waitValues[]			= { wait_value };
		VkPipelineStageFlags	waitStageMasks[]		= { ConvertToVkPipelineStageFlags(wait_stage_mask) };
		u32						waitCount				= wait_value != Fence::InvalidValue;

		VkSemaphore				signalSemaphores[]		= { static_cast<VulkanFence*>(fence)->GetSemaphore() };
		u64						signalValues[]			= { signal_value };
		u32						signalCount				= signal_value != Fence::InvalidValue;

		const VkTimelineSemaphoreSubmitInfo timelineSubmitInfo {
			.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
			.waitSemaphoreValueCount = waitCount,
			.pWaitSemaphoreValues = waitValues,
			.signalSemaphoreValueCount = signalCount,
			.pSignalSemaphoreValues = signalValues,
		};

		const VkSubmitInfo submitInfo {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = &timelineSubmitInfo,
			.waitSemaphoreCount = waitCount,
			.pWaitSemaphores = waitSemaphores,
			.pWaitDstStageMask = waitStageMasks,
			.commandBufferCount = 1,
			.pCommandBuffers = commandBuffer,
			.signalSemaphoreCount = signalCount,
			.pSignalSemaphores = signalSemaphores
		};

		axVerifyFmt(VK_SUCCESS == vkQueueSubmit(m_queue, 1, &submitInfo, nullptr),
			"Failed to submit command buffer!"
		);
	}

	void VulkanQueue::Submit(const QueueSubmitDesc& desc)
	{
		VkCommandBuffer* commandBuffers = static_cast<VkCommandBuffer*>(_alloca(sizeof(VkCommandBuffer) * desc.commandBuffers.size()));
		for (u32 i = 0; i < desc.commandBuffers.size(); i++)
		{
			commandBuffers[i] = static_cast<VkCommandBuffer>(desc.commandBuffers[i]->GetNativeHandle());
		}

		VkSemaphore				waitSemaphores[]		= { static_cast<VulkanFence*>(desc.fence)->GetSemaphore(), m_device->GetImageAcquiredSemaphore() };
		u64						waitValues[]			= { desc.fenceWaitValue, 0 };
		VkPipelineStageFlags	waitStageMasks[]		= { ConvertToVkPipelineStageFlags(desc.fenceWaitStageMask), ConvertToVkPipelineStageFlags(desc.imageAcquiredWaitStageMask) };
		u32						waitCount				= desc.waitImageAcquired ? 2 : 1;

		VkSemaphore				signalSemaphores[]		= { static_cast<VulkanFence*>(desc.fence)->GetSemaphore(), m_device->GetRenderCompleteSemaphore() };
		u64						signalValues[]			= { desc.fenceSignalValue, 0 };
		u32						signalCount				= desc.signalRenderComplete ? 2 : 1;

		const VkTimelineSemaphoreSubmitInfo timelineSubmitInfo {
			.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
			.waitSemaphoreValueCount = waitCount,
			.pWaitSemaphoreValues = waitValues,
			.signalSemaphoreValueCount = signalCount,
			.pSignalSemaphoreValues = signalValues,
		};

		const VkSubmitInfo submitInfo {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = &timelineSubmitInfo,
			.waitSemaphoreCount = waitCount,
			.pWaitSemaphores = waitSemaphores,
			.pWaitDstStageMask = waitStageMasks,
			.commandBufferCount = (u32)desc.commandBuffers.size(),
			.pCommandBuffers = commandBuffers,
			.signalSemaphoreCount = signalCount,
			.pSignalSemaphores = signalSemaphores
		};

		axVerifyFmt(VK_SUCCESS == vkQueueSubmit(m_queue, 1, &submitInfo, nullptr),
			"Failed to submit command buffer!"
		);
	}

	void VulkanQueue::Flush()
	{
		const VkResult result = vkQueueSubmit2(m_queue, m_submitInfos.size(), m_submitInfos.data(), m_device->GetRenderFence());
		if (axVerifyFmt(VK_SUCCESS == result, "Failed to submit command buffer : {}", string_VkResult(result)))
		{
			m_submitInfos.clear();
		}
	}

	void VulkanQueue::Present()
	{
		if (!axVerifyFmt(CanPresent(), "This queue cannot present to the swapchain!"))
		{
			return;
		}

		VkSemaphore waitSemaphores[] = { m_device->GetRenderCompleteSemaphore() };
		u32 swapchainImageIndices[] = { m_device->GetCurrentSwapchainImageIndex() };

		const VkPresentInfoKHR presentInfo {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = (u32)std::size(waitSemaphores),
			.pWaitSemaphores = waitSemaphores,
			.swapchainCount = 1,
			.pSwapchains = &m_device->GetSwapchain().handle,
			.pImageIndices = swapchainImageIndices,
			.pResults = nullptr,
		};

		const VkResult result = vkQueuePresentKHR(m_queue, &presentInfo);
		if (VK_ERROR_OUT_OF_DATE_KHR == result)
		{
			m_device->GetContext()->ResizeSurface();
			return;
		}
		axVerifyFmt(VK_SUCCESS == result || VK_SUBOPTIMAL_KHR == result, "Failed to present swapchain image : {}", string_VkResult(result));
	}

	void VulkanQueue::WaitForIdle()
	{
		vkQueueWaitIdle(m_queue);
	}

	// Vulkan Command Buffer
	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		vkFreeCommandBuffers(m_device->GetLogicalDevice(), m_commandPool, 1, &m_commandBuffer);
	}

	void VulkanCommandBuffer::Begin()
	{
		const VkCommandBufferBeginInfo beginInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
		};

		axVerifyFmt(VK_SUCCESS == vkBeginCommandBuffer(m_commandBuffer, &beginInfo),
			"Failed to begin command buffer recording!"
		);
	}

	void VulkanCommandBuffer::End()
	{
		axVerifyFmt(VK_SUCCESS == vkEndCommandBuffer(m_commandBuffer),
			"Failed to record command buffer!"
		);
	}

	void VulkanCommandBuffer::BindGlobalDescriptorSets()
	{
		auto bindlessDescriptorSets = m_device->GetBindlessDescriptorSets();

		const VkPipelineBindPoint bindPoint = GetBindPointForQueueType(m_queue);

		vkCmdBindDescriptorSets(m_commandBuffer,
			bindPoint,
			m_device->GetBindlessPipelineLayout(),
			0, bindlessDescriptorSets.size(), bindlessDescriptorSets.data(),
			0 , nullptr);
	}

	void VulkanCommandBuffer::BindComputePipeline(ComputePipeline const* pipeline)
	{
		VulkanComputePipeline const* vkPipeline = static_cast<VulkanComputePipeline const*>(pipeline);

		vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vkPipeline->GetNativeHandle());
	}

	void VulkanCommandBuffer::Dispatch(Dim3D group_counts)
	{
		vkCmdDispatch(m_commandBuffer, group_counts.x, group_counts.y, group_counts.z);
	}

	void VulkanCommandBuffer::BeginRendering(ImageView const* color_image_view, ImageView const* depth_stencil_image_view, bool clear)
	{
		VkRenderingAttachmentInfo colorAttachmentInfo {
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = {
				.color = { 0.01f, 0.01f, 0.01f, 1.0f },
			}
		};

		VkRenderingAttachmentInfo depthStencilAttachmentInfo {
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			.loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = {
				.depthStencil = { .depth = 0.0, .stencil = 0 }
			}
		};

		VkRenderingInfo renderingInfo {
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.renderArea = { .offset = { 0, 0}, .extent = m_device->GetSwapchain().extent },
			.layerCount = 1,
		};

		if (color_image_view)
		{
			colorAttachmentInfo.imageView = static_cast<const VulkanImageView*>(color_image_view)->GetNativeHandle();
			renderingInfo.colorAttachmentCount = 1;
			renderingInfo.pColorAttachments = &colorAttachmentInfo;
		}

		if (depth_stencil_image_view)
		{
			depthStencilAttachmentInfo.imageView = static_cast<const VulkanImageView*>(depth_stencil_image_view)->GetNativeHandle();
			renderingInfo.pDepthAttachment = &depthStencilAttachmentInfo;
		}

		vkCmdBeginRendering(m_commandBuffer, &renderingInfo);
	}

	void VulkanCommandBuffer::EndRendering()
	{
		vkCmdEndRendering(m_commandBuffer);
	}

	void VulkanCommandBuffer::BindGraphicsPipeline(GraphicsPipeline const* pipeline)
	{
		VulkanGraphicsPipeline const* vkPipeline = static_cast<VulkanGraphicsPipeline const*>(pipeline);

		vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline->GetNativeHandle());
	}

	void VulkanCommandBuffer::BindDescriptorSet(DescriptorSet const& descriptor_set, GraphicsPipeline const* pipeline)
	{
		const VkDescriptorSet vkDescriptorSets[] = { static_cast<VkDescriptorSet>(descriptor_set.GetNativeHandle()) };

		vkCmdBindDescriptorSets(m_commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			static_cast<VulkanGraphicsPipeline const*>(pipeline)->GetPipelineLayout(),
			0, 1, vkDescriptorSets,
			0, nullptr);
	}

	void VulkanCommandBuffer::PushConstants(AxArrayRef<const char> const& bytes)
	{
		vkCmdPushConstants(m_commandBuffer,
			m_device->GetBindlessPipelineLayout(),
			VK_SHADER_STAGE_ALL,
			0, bytes.size(), bytes.data());
	}

	void VulkanCommandBuffer::SetViewport(Viewport viewport)
	{
		const VkViewport vkViewport {
			.x = viewport.x,
			.y = viewport.y,
			.width = viewport.width,
			.height = viewport.height,
			.minDepth = viewport.minDepth,
			.maxDepth = viewport.maxDepth,
		};
		vkCmdSetViewport(m_commandBuffer, 0, 1, &vkViewport);
	}

	void VulkanCommandBuffer::SetScissor(Rect2D scissor)
	{
		const VkRect2D vkScissor {
			.offset = { .x = scissor.x, .y = scissor.y },
			.extent = { .width = scissor.width, .height = scissor.height },
		};
		vkCmdSetScissor(m_commandBuffer, 0, 1, &vkScissor);
	}

	void VulkanCommandBuffer::BindVertexBuffer(Buffer const* buffer)
	{
		VulkanBuffer const* vkbuffer = static_cast<VulkanBuffer const*>(buffer);

		VkBuffer buffers[] = { vkbuffer->GetNativeHandle() };
		VkDeviceSize offsets[] = { 0 };
		//VkDeviceSize sizes[] = { vkbuffer->GetSize() };
		//VkDeviceSize strides[] = { vkbuffer->GetStride() };

		vkCmdBindVertexBuffers(m_commandBuffer, 0, 1, buffers, offsets);
		// vkCmdBindVertexBuffers2(m_commandBuffer, 0, 1, buffers, offsets, sizes, strides);
	}

	void VulkanCommandBuffer::BindIndexBuffer(Buffer const* buffer)
	{
		VulkanBuffer const* vkbuffer = static_cast<VulkanBuffer const*>(buffer);

		vkCmdBindIndexBuffer(m_commandBuffer, vkbuffer->GetNativeHandle(), 0, VK_INDEX_TYPE_UINT32);
	}

	void VulkanCommandBuffer::Draw(u32 vertex_count)
	{
		vkCmdDraw(m_commandBuffer, vertex_count, 1, 0, 0);
	}

	void VulkanCommandBuffer::DrawIndexed(u32 index_count)
	{
		vkCmdDrawIndexed(m_commandBuffer, index_count, 1, 0, 0, 0);
	}

	void VulkanCommandBuffer::TransitionImage(const Image* image,
											  ImageLayout old_layout, PipelineStageFlags src_stage_mask, AccessFlags src_access_mask, QueueType src_queue,
											  ImageLayout new_layout, PipelineStageFlags dst_stage_mask, AccessFlags dst_access_mask, QueueType dst_queue)
	{
		const VkImageAspectFlags aspectMask = IsDepthFormat(static_cast<VulkanImage const*>(image)->GetFormat()) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

		const VkImageMemoryBarrier2 imageBarrier {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = ConvertToVkPipelineStageFlags(src_stage_mask),
			.srcAccessMask = ConvertToVkAccessFlags(src_access_mask),
			.dstStageMask = ConvertToVkPipelineStageFlags(dst_stage_mask),
			.dstAccessMask = ConvertToVkAccessFlags(dst_access_mask),
			.oldLayout = ConvertToVkImageLayout(old_layout),
			.newLayout = ConvertToVkImageLayout(new_layout),
			.srcQueueFamilyIndex = static_cast<const VulkanQueue*>(m_device->GetQueue(src_queue))->GetQueueFamilyIndex(),
			.dstQueueFamilyIndex = static_cast<const VulkanQueue*>(m_device->GetQueue(dst_queue))->GetQueueFamilyIndex(),
			.image = static_cast<const VulkanImage*>(image)->GetNativeHandle(),
			.subresourceRange = {
				.aspectMask = aspectMask,
				.baseMipLevel = 0,
				.levelCount = VK_REMAINING_MIP_LEVELS,
				.baseArrayLayer = 0,
				.layerCount = VK_REMAINING_ARRAY_LAYERS,
			},
		};

		const VkDependencyInfo dependencyInfo {
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		    .imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &imageBarrier,
		};

		vkCmdPipelineBarrier2(m_commandBuffer, &dependencyInfo);
	}

	void VulkanCommandBuffer::InsertMemoryBarrier(PipelineStageFlags src_stage_mask, AccessFlags src_access_mask,
	                                              PipelineStageFlags dst_stage_mask, AccessFlags dst_access_mask)
	{
		const VkMemoryBarrier2 memoryBarrier {
			.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
			.srcStageMask = ConvertToVkPipelineStageFlags(src_stage_mask),
			.srcAccessMask = ConvertToVkAccessFlags(src_access_mask),
			.dstStageMask = ConvertToVkPipelineStageFlags(dst_stage_mask),
			.dstAccessMask = ConvertToVkAccessFlags(dst_access_mask)
		};

		const VkDependencyInfo dependencyInfo {
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		    .memoryBarrierCount = 1,
		    .pMemoryBarriers = &memoryBarrier
		};

		vkCmdPipelineBarrier2(m_commandBuffer, &dependencyInfo);
	}

	void VulkanCommandBuffer::BlitImage(const Image* src, ImageLayout src_layout,
	                                    const Image* dst, ImageLayout dst_layout)
	{
		const VulkanImage* vksrc = static_cast<const VulkanImage*>(src);
		const VulkanImage* vkdst = static_cast<const VulkanImage*>(dst);

		const VkExtent3D srcExtent = vksrc->GetExtent();

		const VkImageBlit regions = {
			.srcSubresource = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.srcOffsets = {
				{ 0, 0, 0 }, 
				{ (s32)srcExtent.width, (s32)srcExtent.height, (s32)srcExtent.depth }
			},
			.dstSubresource = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.dstOffsets = {
				{ 0, 0, 0 }, 
				{ (s32)srcExtent.width, (s32)srcExtent.height, (s32)srcExtent.depth }
			}
		};

		vkCmdBlitImage(m_commandBuffer, vksrc->GetNativeHandle(), ConvertToVkImageLayout(src_layout), vkdst->GetNativeHandle(), ConvertToVkImageLayout(dst_layout), 1, &regions, VK_FILTER_LINEAR);
	}

	void VulkanCommandBuffer::CopyBuffer(const Buffer* src, const Buffer* dst)
	{
		const VulkanBuffer* vksrc = static_cast<const VulkanBuffer*>(src);
		const VulkanBuffer* vkdst = static_cast<const VulkanBuffer*>(dst);

		const VkBufferCopy copyRegion {
			.srcOffset = 0,
			.dstOffset = 0,
			.size = min(vkdst->GetSize(), vksrc->GetSize())
		};

		vkCmdCopyBuffer(m_commandBuffer, vksrc->GetNativeHandle(), vkdst->GetNativeHandle(), 1, &copyRegion);
	}

	void VulkanCommandBuffer::CopyBufferToImage(const Buffer* src, const Image* dst, ImageLayout layout)
	{
		const VulkanBuffer* vksrc = static_cast<const VulkanBuffer*>(src);
		const VulkanImage* vkdst = static_cast<const VulkanImage*>(dst);

		const VkBufferImageCopy copyRegion {
			.bufferOffset = 0,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,

			.imageSubresource = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.imageOffset = { 0, 0, 0 },
			.imageExtent = vkdst->GetExtent()
		};

		vkCmdCopyBufferToImage(m_commandBuffer, vksrc->GetNativeHandle(), vkdst->GetNativeHandle(), ConvertToVkImageLayout(layout), 1, &copyRegion);
	}

	void VulkanCommandBuffer::PushLabel(const char* label_str, math::Vector4 const& color)
	{
		const VkDebugUtilsLabelEXT label {
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
			.pLabelName = label_str,
			.color = { color[0], color[1], color[2], color[3] },
		};

		vkdbg.CmdBeginDebugUtilsLabel(m_commandBuffer, &label);
	}

	void VulkanCommandBuffer::PopLabel()
	{
		vkdbg.CmdEndDebugUtilsLabel(m_commandBuffer);
	}

	// Vulkan Buffer
	VulkanBuffer::~VulkanBuffer()
	{
		m_device->DestroyBuffer(this);
	}

	// Vulkan Image
	VulkanImage::VulkanImage(VulkanDevice const* device, VkImage image, VkImageCreateInfo const& create_info, VkImageView view)
	: m_device(device), m_image(image), m_allocation(nullptr), m_allocationInfo(), m_view(apex_new VulkanImageView(view, this))
	, m_extent(create_info.extent), m_usage(create_info.usage), m_format(create_info.format)
	{
	}

	VulkanImage::VulkanImage(VulkanDevice const* device, VkImage image, VkImageCreateInfo const& create_info, VmaAllocation allocation, VmaAllocationInfo const& allocation_info, VkImageView view)
	: m_device(device), m_image(image), m_allocation(allocation), m_allocationInfo(allocation_info), m_view(apex_new VulkanImageView(view, this))
	, m_extent(create_info.extent), m_usage(create_info.usage), m_format(create_info.format)
	{}

	VulkanImage::VulkanImage(VulkanDevice const* device, VkImage image, VkImageView view, VkExtent3D extent, VkFormat format, VkImageUsageFlags usage)
	: m_device(device), m_image(image), m_allocation(nullptr), m_allocationInfo(), m_view(apex_new VulkanImageView(view, this))
	, m_extent(extent), m_usage(usage), m_format(format)
	{}

	VulkanImage::~VulkanImage()
	{
		delete m_view;
		m_device->DestroyImage(this);
	}

	// Vulkan Image View
	VulkanImageView::~VulkanImageView()
	{
		if (m_owner)
			m_owner->GetDevice()->DestroyImageView(this);
	}

	// Vulkan Shader Module
	VulkanShaderModule::~VulkanShaderModule()
	{
		m_device->DestroyShaderModule(this);
	}

	// Vulkan Pipelines
	VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
	{
		m_device->DestroyPipeline(this);
	}

	VulkanComputePipeline::~VulkanComputePipeline()
	{
		m_device->DestroyPipeline(this);
	}

	// Vulkan Fence
	VulkanFence::~VulkanFence()
	{
		m_device->DestroyFence(this);
	}

	void VulkanFence::Signal(u64 value)
	{
		const VkSemaphoreSignalInfo signalInfo {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
			.semaphore = m_semaphore,
			.value = value,
		};

		axVerifyFmt(VK_SUCCESS == vkSignalSemaphore(m_device->GetLogicalDevice(), &signalInfo),
			"Failed to signal semaphore!"
		);

		m_counter = value;
	}

	void VulkanFence::Wait(u64 value)
	{
		const VkSemaphoreWaitInfo waitInfo {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.semaphoreCount = 1,
			.pSemaphores = &m_semaphore,
			.pValues = &value,
		};

		axVerifyFmt(VK_SUCCESS == vkWaitSemaphores(m_device->GetLogicalDevice(), &waitInfo, Constants::u64_MAX),
			"Failed to wait for semaphore!"
		);
	}

	VulkanContextImpl::VulkanContextImpl(
#if APEX_PLATFORM_WIN32
		HINSTANCE hinstance, HWND hwnd
#endif // APEX_PLATFORM_WIN32
	)
	{
		axInfo("Initializing Vulkan context...");

		const VkApplicationInfo applicationInfo {
			.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName   = "Game",
			.applicationVersion = VK_MAKE_VERSION(0, 0, 1),
			.pEngineName        = "Apex Game Engine",
			.engineVersion      = VK_MAKE_VERSION(0, 2, 1),
			.apiVersion         = VK_API_VERSION_1_3
		};

		const char* kRequiredInstanceExtensions[] = {
			// Window creation extensions
		#if APEX_PLATFORM_WIN32
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		#endif

			// Debug utilities extension. Must be last in the list
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
		};

		const char* kValidationLayerNames[] = {
			"VK_LAYER_KHRONOS_validation",
		};

		axAssertFmt(CheckValidationLayerSupport(kValidationLayerNames, std::size(kValidationLayerNames)),
			"Validation layers are requested but not supported!"
		);

		// Enable Synchronization validation
		const VkBool32 verbose_value = true;
		const VkLayerSettingEXT layerSetting { "VK_LAYER_KHRONOS_validation", "validate_sync", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &verbose_value };
		VkLayerSettingsCreateInfoEXT layerSettingsCreateInfo { VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &layerSetting };

		const VkInstanceCreateInfo instanceCreateInfo {
			.sType				     = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext				     = &layerSettingsCreateInfo,
			.pApplicationInfo	     = &applicationInfo,
			.enabledLayerCount       = static_cast<u32>(std::size(kValidationLayerNames)),
			.ppEnabledLayerNames     = kValidationLayerNames,
			.enabledExtensionCount   = static_cast<u32>(std::size(kRequiredInstanceExtensions)),
			.ppEnabledExtensionNames = kRequiredInstanceExtensions,
		};

		axVerifyFmt(VK_SUCCESS == vkCreateInstance(&instanceCreateInfo, VK_NULL_HANDLE, &m_instance),
			"Failed to create Vulkan instance!"
		);
		axDebug("Vulkan Instance created");

	#if APEX_PLATFORM_WIN32
		CreateSurface(hinstance, hwnd);
		axDebug("Vulkan Win32 surface created");
	#endif

		VulkanPhysicalDeviceFeatures requiredDeviceFeatures {};
		// Vulkan 1.2 features
		requiredDeviceFeatures.features12.bufferDeviceAddress = true;
		requiredDeviceFeatures.features12.descriptorIndexing = true;
		requiredDeviceFeatures.features12.descriptorBindingPartiallyBound = true;
		requiredDeviceFeatures.features12.descriptorBindingVariableDescriptorCount = true;
		requiredDeviceFeatures.features12.runtimeDescriptorArray = true;
		requiredDeviceFeatures.features12.timelineSemaphore = true;
		// shader resource arrays non-uniform indexing
		requiredDeviceFeatures.features12.shaderUniformBufferArrayNonUniformIndexing = true;
		requiredDeviceFeatures.features12.shaderSampledImageArrayNonUniformIndexing = true;
		requiredDeviceFeatures.features12.shaderStorageImageArrayNonUniformIndexing = true;
		requiredDeviceFeatures.features12.shaderStorageBufferArrayNonUniformIndexing = true;
		// descriptor bindings update after bind
		requiredDeviceFeatures.features12.descriptorBindingUniformBufferUpdateAfterBind = true;
		requiredDeviceFeatures.features12.descriptorBindingSampledImageUpdateAfterBind = true;
		requiredDeviceFeatures.features12.descriptorBindingStorageImageUpdateAfterBind = true;
		requiredDeviceFeatures.features12.descriptorBindingStorageBufferUpdateAfterBind = true;
		// Vulkan 1.3 features
		requiredDeviceFeatures.features13.synchronization2 = true;
		requiredDeviceFeatures.features13.dynamicRendering = true;
		requiredDeviceFeatures.features13.maintenance4 = true;

		VkPhysicalDeviceShaderAtomicFloatFeaturesEXT atomicFloatFeatures {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT,
			.pNext = nullptr,
			.shaderBufferFloat32AtomicAdd = VK_TRUE,
		};
		requiredDeviceFeatures.SetupChain(&atomicFloatFeatures);

		const VkPhysicalDevice physicalDevice = SelectPhysicalDevice(requiredDeviceFeatures);

		CreateDevice(physicalDevice, requiredDeviceFeatures);
		if (!axVerifyFmt(m_device, "Vulkan logical device could not be created!"))
		{
			return;
		}

		// Enable Debug Messenger
		{
			const VkDebugUtilsMessageSeverityFlagsEXT messageSeverityFlags =
	#if GFX_LOGGING_VERBOSE
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
	#endif // GFX_LOGGING_VERBOSE
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

			const VkDebugUtilsMessageTypeFlagsEXT messageTypeFlags =
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
				//VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;

			const VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo {
				.sType				= VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
				.pNext				= nullptr,
			    .messageSeverity	= messageSeverityFlags,
			    .messageType		= messageTypeFlags,
			    .pfnUserCallback	= DebugMessengerCallback,
			    .pUserData			= this,
			};

			auto getInstanceProcAddrOrDefault = []<typename Fn>(auto instance, const char* fname) -> Fn
			{
				Fn fn = (Fn)vkGetInstanceProcAddr(instance, fname);
				if (fn == nullptr)
				{
					axWarnFmt("Could not load function {}", fname);
					fn = (Fn)+[]() { return VK_RESULT_MAX_ENUM; };
				}
				return fn;
			};

#define GetInstanceProcAddrOrDefault(instance, func) getInstanceProcAddrOrDefault.operator()<CONCAT(PFN_,func)>((instance), STR(func))

			vkdbg.CreateDebugUtilsMessenger =	GetInstanceProcAddrOrDefault(m_instance, vkCreateDebugUtilsMessengerEXT);
			vkdbg.DestroyDebugUtilsMessenger =	GetInstanceProcAddrOrDefault(m_instance, vkDestroyDebugUtilsMessengerEXT);
			vkdbg.SetDebugUtilsObjectName =		GetInstanceProcAddrOrDefault(m_instance, vkSetDebugUtilsObjectNameEXT);
			vkdbg.QueueBeginDebugUtilsLabel =	GetInstanceProcAddrOrDefault(m_instance, vkQueueBeginDebugUtilsLabelEXT);
			vkdbg.QueueEndDebugUtilsLabel =		GetInstanceProcAddrOrDefault(m_instance, vkQueueEndDebugUtilsLabelEXT);
			vkdbg.CmdBeginDebugUtilsLabel =		GetInstanceProcAddrOrDefault(m_instance, vkCmdBeginDebugUtilsLabelEXT);
			vkdbg.CmdEndDebugUtilsLabel =		GetInstanceProcAddrOrDefault(m_instance, vkCmdEndDebugUtilsLabelEXT);

#undef GetInstanceProcAddrOrDefault
			
			axVerifyFmt(VK_SUCCESS == vkdbg.CreateDebugUtilsMessenger(m_instance, &debugUtilsMessengerCreateInfo, VK_NULL_HANDLE, &m_debugUtilsMessenger),
				"Failed to create debug utils messenger!"
			);
		}

	#if APEX_PLATFORM_WIN32
		RECT rect;
		GetClientRect(hwnd, &rect);
		const u32 width = rect.right - rect.left;
		const u32 height = rect.bottom - rect.top;
	#endif
		m_device->CreateSwapchain(m_surface, width, height);
		m_device->CreateCommandPools();
	}

	VulkanContextImpl::~VulkanContextImpl()
	{
		m_device.reset();
	#if APEX_PLATFORM_WIN32
		vkDestroySurfaceKHR(m_instance, m_surface, VK_NULL_HANDLE);
	#endif // APEX_PLATFORM_WIN32
		vkdbg.DestroyDebugUtilsMessenger(m_instance, m_debugUtilsMessenger, VK_NULL_HANDLE);
		vkDestroyInstance(m_instance, VK_NULL_HANDLE);
	}


#if APEX_PLATFORM_WIN32
	void VulkanContextImpl::CreateSurface(HINSTANCE hinstance, HWND hwnd)
	{
		const VkWin32SurfaceCreateInfoKHR surfaceCreateInfo {
			.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.hinstance = hinstance,
			.hwnd = hwnd
		};

		axVerifyFmt(VK_SUCCESS == vkCreateWin32SurfaceKHR(m_instance, &surfaceCreateInfo, nullptr, &m_surface),
			"Failed to create Vulkan Win32 surface!"
		);
	}

	VkPhysicalDevice VulkanContextImpl::SelectPhysicalDevice(VulkanPhysicalDeviceFeatures const& required_device_features) const
	{
		u32 physicalDeviceCount;
		vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr);

		axAssertFmt(physicalDeviceCount > 0, "Failed to find GPUs with Vulkan support!");

		AxArray<VkPhysicalDevice> devices;
		devices.resize(physicalDeviceCount);
		vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, devices.data());

		VkPhysicalDevice physicalDevice {};

		for (VkPhysicalDevice device : devices)
		{
			if (IsPhysicalDeviceSuitable(device, m_surface, required_device_features))
			{
				physicalDevice = device;
				break;
			}
		}
		axAssertFmt(physicalDevice, "Failed to find a suitable GPU!");

		return physicalDevice;
	}

	void VulkanContextImpl::CreateDevice(VkPhysicalDevice physical_device, VulkanPhysicalDeviceFeatures const& enabled_device_features)
	{
		VulkanQueueInfo queueInfos[QueueType::COUNT];
		FindQueueFamilies(physical_device, m_surface, true, queueInfos);

		axAssertFmt(queueInfos[QueueType::Graphics].familyIndex != VulkanQueueInfo::kInvalidQueueFamilyIndex, "Required queue indices not found : 'graphics'");
		axAssertFmt(queueInfos[QueueType::Compute ].familyIndex != VulkanQueueInfo::kInvalidQueueFamilyIndex, "Required queue indices not found : 'compute'");
		axAssertFmt(queueInfos[QueueType::Transfer].familyIndex != VulkanQueueInfo::kInvalidQueueFamilyIndex, "Required queue indices not found : 'transfer'");

		AxArray<VkExtensionProperties> availableExtensions;
		u32 extensionCount;
		vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensionCount, nullptr);
		availableExtensions.resize(extensionCount);
		vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensionCount, availableExtensions.dataMutable());

		const char* requiredLayerNames[] = {
			"VK_LAYER_KHRONOS_validation"
		};

		const char* requiredDeviceExtensionNames[] = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME,
			VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
		};

		const char* optionalDeviceExtensionNames[] = {
			VK_EXT_DEVICE_ADDRESS_BINDING_REPORT_EXTENSION_NAME,
		};

		AxArray<const char*> enabledDeviceExtenionNames(availableExtensions.size());

		for (auto extensionName : requiredDeviceExtensionNames)
		{
			auto it = std::find_if(availableExtensions.begin(), availableExtensions.end(), [extensionName](const VkExtensionProperties& prop)
			{
				return strcmp(prop.extensionName, extensionName);
			});

			if (!axVerifyFmt(it != availableExtensions.end(), "Required device extension not available : {}", extensionName))
			{
				return;
			}

			enabledDeviceExtenionNames.append(extensionName);
		}

		for (auto extensionName : optionalDeviceExtensionNames)
		{
			auto it = std::find_if(availableExtensions.begin(), availableExtensions.end(), [extensionName](const VkExtensionProperties& prop)
			{
				return strcmp(prop.extensionName, extensionName);
			});

			if (axVerifyFmt(it != availableExtensions.end(), "Required device extension not available : {}", extensionName))
			{
				enabledDeviceExtenionNames.append(extensionName);
			}
		}

		// Create queues
		const size_t numQueues = QueueType::COUNT;
		VkDeviceQueueCreateInfo queueCreateInfos[numQueues];
		const float queuePriority = 1.f;
		for (u32 i = 0; i < numQueues; i++)
		{
			queueCreateInfos[i] = VkDeviceQueueCreateInfo {
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = queueInfos[i].familyIndex,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority,
			};
		}

		const VkDeviceCreateInfo deviceCreateInfo {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &enabled_device_features,
			.queueCreateInfoCount = numQueues,
			.pQueueCreateInfos = queueCreateInfos,
			.enabledLayerCount = (u32)std::size(requiredLayerNames),
			.ppEnabledLayerNames = requiredLayerNames,
			.enabledExtensionCount = (u32)std::size(requiredDeviceExtensionNames),
			.ppEnabledExtensionNames = requiredDeviceExtensionNames,
		};

		VkDevice logicalDevice;
		const VkResult result = vkCreateDevice(physical_device, &deviceCreateInfo, nullptr, &logicalDevice);
		if (!axVerifyFmt(VK_SUCCESS == result, "Failed to create Vulkan logical device : {}", string_VkResult(result)))
		{
			return;
		}

		const VmaAllocatorCreateInfo allocatorCreateInfo {
			.flags = 0,
			.physicalDevice = physical_device,
			.device = logicalDevice,
			.instance = m_instance,
			.vulkanApiVersion = VK_API_VERSION_1_3,
		};
		VmaAllocator allocator;

		if (!axVerifyFmt(VK_SUCCESS == vmaCreateAllocator(&allocatorCreateInfo, &allocator), "Failed to create Vulkan Memory Allocator!"))
		{
			return;
		}

		m_device = apex::make_unique<VulkanDevice>(this, physical_device, logicalDevice, allocator, make_array_ref(queueInfos));
	}

	bool VulkanContextImpl::ResizeSurface()
	{
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->m_physicalDevice, m_surface, &surfaceCapabilities);

		return ResizeSurface(surfaceCapabilities.currentExtent.width, surfaceCapabilities.currentExtent.height);
	}

	bool VulkanContextImpl::ResizeSurface(u32 width, u32 height) const
	{
		if (width == m_device->m_swapchain.extent.width &&
			height == m_device->m_swapchain.extent.height)
		{
			return false;
		}

		m_device->WaitForIdle();

		m_device->CreateSwapchain(m_surface, width, height);

		return true;
	}
#endif // APEX_PLATFORM_WIN32

}

