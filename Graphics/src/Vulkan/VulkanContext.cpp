#include "Graphics/Vulkan/VulkanContext.h"

#include <vulkan/vk_enum_string_helper.h>

#include "Core/Logging.h"
#include "Core/Asserts.h"
#include "Core/Files.h"
#include "Memory/AxHandle.h"
#include "Graphics/Factory.h"
#include "Memory/MemoryManager.h"

namespace apex::gfx {

	namespace vk {

		PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT;
		PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT;
		PFN_vkSetDebugUtilsObjectNameEXT SetDebugUtilsObjectNameEXT;
	}

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

		bool ResizeWindow(u32 width, u32 height) const;

		VkInstance		    GetInstance() const { return m_instance; }
		VkSurfaceKHR	    GetSurface() const	{ return m_surface; }
		VulkanDevice const& GetDevice() const   { return *m_device; }

	private:
		VkInstance               m_instance {};
		VkSurfaceKHR             m_surface {};
		UniquePtr<VulkanDevice>  m_device {};
		VkDebugUtilsMessengerEXT m_debugUtilsMessenger {};
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

		VulkanQueueInfo& graphicsQueue = queueInfos[VulkanQueueFamily::Graphics];
		VulkanQueueInfo& computeQueue = queueInfos[VulkanQueueFamily::Compute];
		VulkanQueueInfo& transferQueue = queueInfos[VulkanQueueFamily::Transfer];

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

		vk::SetDebugUtilsObjectNameEXT(device, &objectNameInfo);
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
		case ImageFormat::R8_UNORM:          return VK_FORMAT_R8_UNORM;
		case ImageFormat::R8_SNORM:          return VK_FORMAT_R8_SNORM;
		case ImageFormat::R8_UINT:           return VK_FORMAT_R8_UINT;
		case ImageFormat::R8_SINT:           return VK_FORMAT_R8_SINT;
		case ImageFormat::R8_SRGB:           return VK_FORMAT_R8_SRGB;
		case ImageFormat::R8G8_UNORM:        return VK_FORMAT_R8G8_UNORM;
		case ImageFormat::R8G8_SNORM:        return VK_FORMAT_R8G8_SNORM;
		case ImageFormat::R8G8_UINT:         return VK_FORMAT_R8G8_UINT;
		case ImageFormat::R8G8_SINT:         return VK_FORMAT_R8G8_SINT;
		case ImageFormat::R8G8_SRGB:         return VK_FORMAT_R8G8_SRGB;
		case ImageFormat::R8G8B8_UNORM:      return VK_FORMAT_R8G8B8_UNORM;
		case ImageFormat::R8G8B8_SNORM:      return VK_FORMAT_R8G8B8_SNORM;
		case ImageFormat::R8G8B8_UINT:       return VK_FORMAT_R8G8B8_UINT;
		case ImageFormat::R8G8B8_SINT:       return VK_FORMAT_R8G8B8_SINT;
		case ImageFormat::R8G8B8_SRGB:       return VK_FORMAT_R8G8B8_SRGB;
		case ImageFormat::B8G8R8_UNORM:      return VK_FORMAT_B8G8R8_UNORM;
		case ImageFormat::B8G8R8_SNORM:      return VK_FORMAT_B8G8R8_SNORM;
		case ImageFormat::B8G8R8_UINT:       return VK_FORMAT_B8G8R8_UINT;
		case ImageFormat::B8G8R8_SINT:       return VK_FORMAT_B8G8R8_SINT;
		case ImageFormat::B8G8R8_SRGB:       return VK_FORMAT_B8G8R8_SRGB;
		case ImageFormat::R8G8B8A8_UNORM:    return VK_FORMAT_R8G8B8A8_UNORM;
		case ImageFormat::R8G8B8A8_SNORM:    return VK_FORMAT_R8G8B8A8_SNORM;
		case ImageFormat::R8G8B8A8_UINT:     return VK_FORMAT_R8G8B8A8_UINT;
		case ImageFormat::R8G8B8A8_SINT:     return VK_FORMAT_R8G8B8A8_SINT;
		case ImageFormat::R8G8B8A8_SRGB:     return VK_FORMAT_R8G8B8A8_SRGB;
		case ImageFormat::B8G8R8A8_UNORM:    return VK_FORMAT_B8G8R8A8_UNORM;
		case ImageFormat::B8G8R8A8_SNORM:    return VK_FORMAT_B8G8R8A8_SNORM;
		case ImageFormat::B8G8R8A8_UINT:     return VK_FORMAT_B8G8R8A8_UINT;
		case ImageFormat::B8G8R8A8_SINT:     return VK_FORMAT_B8G8R8A8_SINT;
		case ImageFormat::B8G8R8A8_SRGB:     return VK_FORMAT_B8G8R8A8_SRGB;
		case ImageFormat::D16_UNORM:         return VK_FORMAT_D16_UNORM;
		case ImageFormat::D32_SFLOAT:        return VK_FORMAT_D32_SFLOAT;
		case ImageFormat::S8_UINT:           return VK_FORMAT_S8_UINT;
		case ImageFormat::D16_UNORM_S8_UINT: return VK_FORMAT_D16_UNORM_S8_UINT;
		case ImageFormat::D24_UNORM_S8_UINT: return VK_FORMAT_D24_UNORM_S8_UINT;
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

	VulkanDevice::VulkanDevice(VulkanContextImpl& context, VkPhysicalDevice physical_device, VulkanPhysicalDeviceFeatures const& enabled_device_features)
	{
		m_physicalDevice = physical_device;

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

		// Store the required queue indices
		FindQueueFamilies(m_physicalDevice, context.GetSurface(), true, m_queueInfos);

		axAssertFmt(m_queueInfos[VulkanQueueFamily::Graphics].familyIndex != VulkanQueueInfo::kInvalidQueueFamilyIndex, "Required queue indices not found : 'graphics'");
		axAssertFmt(m_queueInfos[VulkanQueueFamily::Compute].familyIndex != VulkanQueueInfo::kInvalidQueueFamilyIndex, "Required queue indices not found : 'compute'");
		axAssertFmt(m_queueInfos[VulkanQueueFamily::Transfer].familyIndex != VulkanQueueInfo::kInvalidQueueFamilyIndex, "Required queue indices not found : 'transfer'");

		// Create queues
		const u32 numQueues = VulkanQueueFamily::COUNT;
		VkDeviceQueueCreateInfo queueCreateInfos[numQueues];
		float queuePriority = 1.f;
		for (u32 i = 0; i < numQueues; i++)
		{
			queueCreateInfos[i] = VkDeviceQueueCreateInfo {
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = m_queueInfos[i].familyIndex,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority,
			};
		}

		const char* ppEnabledLayerNames[] = { "VK_LAYER_KHRONOS_validation" };
		const char* ppEnabledDeviceExtensionNames[] = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME,
			VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
		};

		const VkDeviceCreateInfo deviceCreateInfo {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &enabled_device_features,
			.queueCreateInfoCount = numQueues,
			.pQueueCreateInfos = queueCreateInfos,
			.enabledLayerCount = 1,
			.ppEnabledLayerNames = ppEnabledLayerNames,
			.enabledExtensionCount = 3,
			.ppEnabledExtensionNames = ppEnabledDeviceExtensionNames,
		};

		axVerifyFmt(VK_SUCCESS == vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_logicalDevice),
					"Failed to create Vulkan logical device!"
		);

		for (u32 i = 0; i < std::size(m_queues); i++)
		{
			vkGetDeviceQueue(m_logicalDevice, m_queueInfos[i].familyIndex, 0, &m_queues[i]);
		}

		const VmaAllocatorCreateInfo allocatorCreateInfo {
			.flags = 0,
			.physicalDevice = m_physicalDevice,
			.device = m_logicalDevice,
			.instance = context.GetInstance(),
			.vulkanApiVersion = VK_API_VERSION_1_3,
		};

		axVerifyFmt(VK_SUCCESS == vmaCreateAllocator(&allocatorCreateInfo, &m_allocator),
					"Failed to create Vulkan Memory Allocator!"
		);

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

	CommandBuffer* VulkanDevice::AllocateCommandBuffer(u32 queueIdx, u32 frame_index, u32 thread_idx) const
	{
		VkCommandBuffer commandBuffer;

		const u32 poolIdx = queueIdx * m_swapchainImageCount * m_renderThreadCount +  frame_index * m_renderThreadCount + thread_idx;

		const VkCommandBufferAllocateInfo allocateInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = m_commandPools[poolIdx],
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};

		axVerifyFmt(VK_SUCCESS == vkAllocateCommandBuffers(m_logicalDevice, &allocateInfo, &commandBuffer),
			"Failed to allocate command buffers!"
		);

		return apex_new (VulkanCommandBuffer)(this, m_commandPools[poolIdx], commandBuffer);
	}

	void VulkanDevice::ResetCommandBuffers(u32 thread_idx) const
	{
		vkResetCommandPool(m_logicalDevice, m_commandPools[m_currentSwapchainImageIndex * m_renderThreadCount + thread_idx], 0);
	}

	void VulkanDevice::ResetCommandBuffers() const
	{
		for (u32 threadIdx = 0; threadIdx < m_renderThreadCount; threadIdx++)
			vkResetCommandPool(m_logicalDevice, m_commandPools[m_currentSwapchainImageIndex * m_renderThreadCount + threadIdx], 0);
	}

	void VulkanDevice::SubmitCommandBuffer(DeviceQueue queue, CommandBuffer* command_buffer /* bool wait_for_image, bool signal_render_complete */) const
	{
		const VkCommandBufferSubmitInfo commandBufferSubmitInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
			.commandBuffer = static_cast<VulkanCommandBuffer*>(command_buffer)->m_commandBuffer,
			.deviceMask = 0,
		};

		const VkSemaphoreSubmitInfo imageAvailableSemaphoreSubmitInfo {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = m_acquireSemaphores[m_currentSwapchainImageIndex],
			.value = 1,
			.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.deviceIndex = 0,
		};

		// TODO: Add other wait semaphores from user
		const VkSemaphoreSubmitInfo waitSemaphoreInfos[] = { imageAvailableSemaphoreSubmitInfo };

		const VkSemaphoreSubmitInfo renderingCompleteSemaphoreSubmitInfo {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = m_releaseSemaphores[m_currentSwapchainImageIndex],
			.value = 1,
			.stageMask = queue == DeviceQueue::Graphics ? VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT : queue == DeviceQueue::Transfer ? VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT : VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
			.deviceIndex = 0,
		};

		// TODO: Add other signal semaphores from user
		const VkSemaphoreSubmitInfo signalSemaphoreInfos[] = { renderingCompleteSemaphoreSubmitInfo };

		const VkSubmitInfo2 submitInfo {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.waitSemaphoreInfoCount = static_cast<uint32_t>(std::size(waitSemaphoreInfos)),
			.pWaitSemaphoreInfos = waitSemaphoreInfos,
			.commandBufferInfoCount = 1,
			.pCommandBufferInfos = &commandBufferSubmitInfo,
			.signalSemaphoreInfoCount = static_cast<uint32_t>(std::size(signalSemaphoreInfos)),
			.pSignalSemaphoreInfos = signalSemaphoreInfos,
		};

		axVerifyFmt(VK_SUCCESS == vkQueueSubmit2(m_queues[queue], 1, &submitInfo, m_renderFences[m_currentSwapchainImageIndex]),
			"Failed to submit command buffer!"
		);
	}

	void VulkanDevice::SubmitImmediateCommandBuffer(DeviceQueue queue, CommandBuffer* command_buffer) const
	{
		VkCommandBuffer commandBuffers[] = { static_cast<VulkanCommandBuffer*>(command_buffer)->m_commandBuffer };

		const VkSubmitInfo submitInfo {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.commandBufferCount = 1,
			.pCommandBuffers = commandBuffers,
		};

		vkQueueSubmit(m_queues[queue], 1, &submitInfo, VK_NULL_HANDLE);
	}

	const Image* VulkanDevice::AcquireNextImage()
	{
		VkSemaphore acquireSemaphore;

		if (m_recycledSemaphores.empty())
		{
			const VkSemaphoreCreateInfo semaphoreCreateInfo { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
			axVerifyFmt(VK_SUCCESS == vkCreateSemaphore(m_logicalDevice, &semaphoreCreateInfo, VK_NULL_HANDLE, &acquireSemaphore),
				"Failed to create semaphore!"
			);
			//axDebug("Created new semaphore");
		}
		else
		{
			acquireSemaphore = m_recycledSemaphores.back();
			m_recycledSemaphores.pop_back();
			//axDebug("Reusing existing semaphore");
		}

		const VkAcquireNextImageInfoKHR acquireNextImageInfo {
			.sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
			.swapchain = m_swapchain.handle,
			.timeout = 120'000'000'000 /* ns */, // 120 s = 2 min
			.semaphore = acquireSemaphore,
			.fence = VK_NULL_HANDLE,
			.deviceMask = 0x1,
		};

		VkResult result = vkAcquireNextImage2KHR(m_logicalDevice, &acquireNextImageInfo, &m_currentSwapchainImageIndex);
		axAssertFmt(VK_SUCCESS == result, "Failed to acquire swapchain image : {}", string_VkResult(result));

		if (VK_SUCCESS != result)
		{
			m_recycledSemaphores.append(acquireSemaphore);
			return nullptr;
		}

		vkWaitForFences(m_logicalDevice, 1, &m_renderFences[m_currentSwapchainImageIndex], true, 120'000'000'000 /* ns */);
		vkResetFences(m_logicalDevice, 1, &m_renderFences[m_currentSwapchainImageIndex]);

		ResetCommandBuffers(); // all threads

		VkSemaphore oldSemaphore = m_acquireSemaphores[m_currentSwapchainImageIndex];
		if (VK_NULL_HANDLE != oldSemaphore)
			m_recycledSemaphores.append(oldSemaphore);
		m_acquireSemaphores[m_currentSwapchainImageIndex] = acquireSemaphore;

		return &m_swapchainImages[m_currentSwapchainImageIndex];
	}

	void VulkanDevice::Present(DeviceQueue queue)
	{
		const VkPresentInfoKHR presentInfo {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &m_releaseSemaphores[m_currentSwapchainImageIndex],
			.swapchainCount = 1,
			.pSwapchains = &m_swapchain.handle,
			.pImageIndices = &m_currentSwapchainImageIndex,
			.pResults = nullptr,
		};

		VkResult result = vkQueuePresentKHR(m_queues[(size_t)queue], &presentInfo);
		axAssertFmt(VK_SUCCESS == result, "Failed to present swapchain image : {}", string_VkResult(result));
	}

	void VulkanDevice::WaitForQueueIdle(DeviceQueue queue) const
	{
		vkQueueWaitIdle(m_queues[(size_t)queue]);
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
			File shaderFile = File::OpenExisting(filepath);
			shaderCode = shaderFile.Read();
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

		const VkDebugUtilsObjectNameInfoEXT nameInfo {
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
			.objectType = VK_OBJECT_TYPE_SHADER_MODULE,
			.objectHandle = reinterpret_cast<u64>(shader),
			.pObjectName = name,
		};

		vk::SetDebugUtilsObjectNameEXT(m_logicalDevice, &nameInfo);

		return apex_new (VulkanShaderModule)(this, shader, reflect);
	}

	GraphicsPipeline* VulkanDevice::CreateGraphicsPipeline(const char* name, GraphicsPipelineCreateDesc const& desc) const
	{
		AxArray<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;
		shaderStageCreateInfos.resize(2);

		const VulkanShaderModule* vertexModule = static_cast<VulkanShaderModule*>(desc.shaderStages.vertexShader);

		shaderStageCreateInfos[0] = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = vertexModule->m_shader,
			.pName = vertexModule->m_reflect.entry_point_name,
		};

		const VulkanShaderModule* fragmentModule = static_cast<VulkanShaderModule*>(desc.shaderStages.fragmentShader);

		shaderStageCreateInfos[1] = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = fragmentModule->m_shader,
			.pName = fragmentModule->m_reflect.entry_point_name,
		};

		VkVertexInputBindingDescription bindingDescription;
		AxArray<VkVertexInputAttributeDescription> attributeDescriptions;
		PopulateVertexInputBindingDescriptionFromSpirv(vertexModule->m_reflect, bindingDescription, attributeDescriptions);

		const VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.vertexBindingDescriptionCount = 1,
			.pVertexBindingDescriptions = &bindingDescription,
			.vertexAttributeDescriptionCount = static_cast<u32>(attributeDescriptions.size()),
			.pVertexAttributeDescriptions = attributeDescriptions.data(),
		};

		const VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
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

		const VkDebugUtilsObjectNameInfoEXT nameInfo {
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
			.objectType = VK_OBJECT_TYPE_PIPELINE,
			.objectHandle = reinterpret_cast<u64>(pipeline),
			.pObjectName = name,
		};

		vk::SetDebugUtilsObjectNameEXT(m_logicalDevice, &nameInfo);

		return apex_new (VulkanGraphicsPipeline)(this, pipeline, pipelineLayout
	#if GFX_USE_BINDLESS_DESCRIPTORS
	#else
			, descriptorSetLayouts
	#endif
			);
	}

	Buffer* VulkanDevice::CreateBuffer(const char* name, BufferCreateDesc const& desc)
	{
		u32 queueFamilyIndices[] = { m_queueInfos[VulkanQueueFamily::Graphics].familyIndex, m_queueInfos[VulkanQueueFamily::Transfer].familyIndex };

		const VkBufferCreateInfo bufferCreateInfo {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = desc.size,
			.usage = ConvertToVkBufferUsageFlags(desc.usageFlags),
			// TODO: Infer the queue usage from the flags
			.sharingMode = VK_SHARING_MODE_CONCURRENT,
			.queueFamilyIndexCount = static_cast<u32>(std::size(queueFamilyIndices)),
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
		
		const VkDebugUtilsObjectNameInfoEXT nameInfo {
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
			.objectType = VK_OBJECT_TYPE_BUFFER,
			.objectHandle = reinterpret_cast<u64>(buffer),
			.pObjectName = name,
		};

		vk::SetDebugUtilsObjectNameEXT(m_logicalDevice, &nameInfo);

		// Add to the bindless descriptors
		if (desc.usageFlags & (BufferUsageFlagBits::Uniform | BufferUsageFlagBits::Storage))
		{
			const u32 bufferIndex = static_cast<u32>(m_buffers.size());
			m_buffers.append(buffer);

			const VkDescriptorBufferInfo bufferInfo { 
				.buffer = buffer,
				.offset = 0,
				.range = VK_WHOLE_SIZE,
			};

			const BindlessDescriptorType bindlessType = (desc.usageFlags & BufferUsageFlagBits::Uniform) ? BindlessDescriptorType::eUniformBuffer : BindlessDescriptorType::eStorageBuffer;
			const VkDescriptorType descriptorType = (desc.usageFlags & BufferUsageFlagBits::Uniform) ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

			const VkWriteDescriptorSet writes[] = {
				{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = m_bindlessDescriptorSets[bindlessType],
					.dstBinding = 0,
					.dstArrayElement = bufferIndex,
					.descriptorCount = 1,
					.descriptorType = descriptorType,
					.pBufferInfo = &bufferInfo,
				}
			};

			vkUpdateDescriptorSets(m_logicalDevice, 1, writes, 0, nullptr);
		}

		return apex_new (VulkanBuffer)(this, buffer, allocation, allocationInfo);
	}

	Buffer* VulkanDevice::CreateVertexBuffer(const char* name, size_t size, const void* initial_data)
	{
		const BufferCreateDesc desc {
			.size = size,
			.usageFlags = BufferUsageFlagBits::Vertex | BufferUsageFlagBits::TransferDst,
			.requiredFlags = MemoryPropertyFlagBits::DeviceLocal,
			.preferredFlags = MemoryPropertyFlagBits::None,
			.memoryFlags = MemoryAllocateFlagBits::None,
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
			.createMapped = true,
		};

		return CreateBuffer(name, desc);
	}

	Image* VulkanDevice::CreateImage(const char* name, ImageCreateDesc const& desc) const
	{
		u32 queueFamilyIndices[] = { m_queueInfos[VulkanQueueFamily::Graphics].familyIndex, m_queueInfos[VulkanQueueFamily::Transfer].familyIndex };

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
		    .sharingMode = VK_SHARING_MODE_CONCURRENT,
		    .queueFamilyIndexCount =  static_cast<u32>(std::size(queueFamilyIndices)),
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

		// Add to bindless descriptors
		if (desc.usageFlags & ImageUsageFlagBits::Sampled)
		{
			const VkDescriptorImageInfo imageInfo { 
				.sampler = m_defaultNearestSampler,
				.imageView = imageView,
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};

			const VkWriteDescriptorSet writes[] = {
				{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = m_bindlessDescriptorSets[BindlessDescriptorType::eSampledImage],
					.dstBinding = 0,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
					.pImageInfo = &imageInfo,
				}
			};

			vkUpdateDescriptorSets(m_logicalDevice, 1, writes, 0, nullptr); // TODO: Buffer multiple descriptor writes and update at once
		}
		
		return apex_new (VulkanImage)(this, image, imageCreateInfo, allocation, allocationInfo, imageView);
	}

	ImageView* VulkanDevice::CreateImageView(const char* name, Image const* image) const
	{
		return nullptr;
	}

	ImageView* VulkanDevice::CreateImageView(const char* name, ImageViewCreateDesc const& desc) const
	{
		return nullptr;
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
		u32 imageCount = swapchainSupportDetails.capabilities.minImageCount + 1;
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
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,

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

			m_swapchainImages.emplace_back(this, images[i], imageView, VkExtent3D{ m_swapchain.extent.width, m_swapchain.extent.height, 1 }, m_swapchain.surfaceFormat.format);
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

		axDebugFmt("Total Sampled Image Descriptors: {}", totalSampledImageDescriptors);
		axDebugFmt("Total Storage Image Descriptors: {}", totalStorageImageDescriptors);
		axDebugFmt("Total Uniform Buffer Descriptors: {}", totalUniformBufferDescriptors);
		axDebugFmt("Total Storage Buffer Descriptors: {}", totalStorageBufferDescriptors);

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

				axVerifyFmt(VK_SUCCESS == vkCreateDescriptorSetLayout(m_logicalDevice, &descriptorSetLayoutCreateInfo, VK_NULL_HANDLE, &m_bindlessDescriptorSetLayouts[BindlessDescriptorType::eSampler]),
					"Failed to create bindless descriptor set layout!"
				);
			}

			/*VkPushConstantRange pushConstantRanges[] = {
				{
					.stageFlags = VK_SHADER_STAGE_ALL,
					.offset = 
				}
			};*/

			const VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
				.setLayoutCount = static_cast<u32>(m_bindlessDescriptorSetLayouts.size()),
				.pSetLayouts = m_bindlessDescriptorSetLayouts.data(),
				.pushConstantRangeCount = 0,
				.pPushConstantRanges = nullptr, // TODO: Add push constant ranges
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
					.pSetLayouts = &m_bindlessDescriptorSetLayouts[BindlessDescriptorType::eSampler],
				};

				axVerifyFmt(VK_SUCCESS == vkAllocateDescriptorSets(m_logicalDevice, &samplerDescriptorSetAllocateInfo, &m_bindlessDescriptorSets[BindlessDescriptorType::eSampler]),
					"Failed to allocate bindless sampler descriptor set!"
				);
			}
		}

		m_buffers.reserve(std::max(totalUniformBufferDescriptors, totalStorageBufferDescriptors));
		m_textures.reserve(std::max(totalSampledImageDescriptors, totalStorageImageDescriptors));
	}

	void VulkanDevice::CreatePerFrameData()
	{
		const VkSemaphoreCreateInfo semaphoreCreateInfo { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

		m_recycledSemaphores.reserve(m_swapchainImageCount + 1);
		for (u32 i = 0; i < m_swapchainImageCount; i++)
		{
			VkSemaphore semaphore {};
			axVerifyFmt(VK_SUCCESS == vkCreateSemaphore(m_logicalDevice, &semaphoreCreateInfo, VK_NULL_HANDLE, &semaphore),
				"Failed to create semaphore!"
			);
			m_recycledSemaphores.append(semaphore);
		}

		m_acquireSemaphores.resize(m_swapchainImageCount, (VkSemaphore)VK_NULL_HANDLE);

		m_releaseSemaphores.reserve(m_swapchainImageCount);
		for (u32 i = 0; i < m_swapchainImageCount; i++)
		{
			VkSemaphore semaphore {};
			axVerifyFmt(VK_SUCCESS == vkCreateSemaphore(m_logicalDevice, &semaphoreCreateInfo, VK_NULL_HANDLE, &semaphore),
				"Failed to create semaphore!"
			);
			m_releaseSemaphores.append(semaphore);
		}

		const VkFenceCreateInfo fenceCreateInfo {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT,
		};

		m_renderFences.reserve(m_swapchainImageCount);
		for (u32 i = 0; i < m_swapchainImageCount; i++)
		{
			VkFence fence {};
			axVerifyFmt(VK_SUCCESS == vkCreateFence(m_logicalDevice, &fenceCreateInfo, VK_NULL_HANDLE, &fence),
				"Failed to create fence!"
			);
			m_renderFences.append(fence);
		}
	}

	void VulkanDevice::DestroyPerFrameData()
	{
		for (VkSemaphore& semaphore : m_recycledSemaphores)
		{
			vkDestroySemaphore(m_logicalDevice, semaphore, VK_NULL_HANDLE);
		}

		for (VkSemaphore& semaphore : m_acquireSemaphores)
		{
			vkDestroySemaphore(m_logicalDevice, semaphore, VK_NULL_HANDLE);
		}

		for (VkSemaphore& semaphore : m_releaseSemaphores)
		{
			vkDestroySemaphore(m_logicalDevice, semaphore, VK_NULL_HANDLE);
		}

		for (VkFence& fence : m_renderFences)
		{
			vkDestroyFence(m_logicalDevice, fence, VK_NULL_HANDLE);
		}

		m_recycledSemaphores.clear();
		m_acquireSemaphores.clear();
		m_releaseSemaphores.clear();
		m_renderFences.clear();
	}

	void VulkanDevice::CreateCommandPools()
    {
		m_commandPools.resize(3 * m_swapchainImageCount * m_renderThreadCount);

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
						.queueFamilyIndex = m_queueInfos[queueIdx].familyIndex,
	                };

	                VkCommandPool commandPool;
	                axVerifyFmt(VK_SUCCESS == vkCreateCommandPool(m_logicalDevice, &poolCreateInfo, nullptr, &commandPool),
						"Failed to create command pool!"
					);

					const u32 poolIdx = queueIdx * m_swapchainImageCount * m_renderThreadCount + frameIdx * m_renderThreadCount + threadIdx;
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

    void VulkanContext::Init(
#if APEX_PLATFORM_WIN32
		HINSTANCE hinstance, HWND hwnd
#endif // APEX_PLATFORM_WIN32
	)
	{
		AxHandle handle = apex::make_handle<VulkanContextImpl>();
		m_pImpl = new (handle) VulkanContextImpl(
#if APEX_PLATFORM_WIN32
		hinstance, hwnd
#endif // APEX_PLATFORM_WIN32
		);
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

	void VulkanContext::ResizeWindow(u32 width, u32 height) const
	{
		(void)m_pImpl->ResizeWindow(width, height);
	}

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

	void VulkanCommandBuffer::BeginRendering(ImageView const* color_image_view, ImageView const* depth_stencil_image_view)
	{
		const VkRenderingAttachmentInfo colorAttachmentInfo {
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.imageView = static_cast<const VulkanImageView*>(color_image_view)->GetNativeHandle(),
			.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = {
				.color = { 0.01f, 0.01f, 0.01f, 1.0f },
			}
		};

		const VkRenderingAttachmentInfo depthStencilAttachmentInfo {
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.imageView = static_cast<const VulkanImageView*>(depth_stencil_image_view)->GetNativeHandle(),
			.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = {
				.depthStencil = { .depth = 0.0, .stencil = 0 }
			}
		};

		const VkRenderingInfo renderingInfo {
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.renderArea = { .offset = { 0, 0}, .extent = m_device->GetSwapchain().extent },
			.layerCount = 1,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentInfo,
			.pDepthAttachment = &depthStencilAttachmentInfo,
			.pStencilAttachment = nullptr,
		};

		vkCmdBeginRendering(m_commandBuffer, &renderingInfo);

		auto bindlessDescriptorSets = m_device->GetBindlessDescriptorSets();

		vkCmdBindDescriptorSets(m_commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_device->GetBindlessPipelineLayout(),
			0, bindlessDescriptorSets.count, bindlessDescriptorSets._data,
			0 , nullptr);
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

	void VulkanCommandBuffer::TransitionImage(const Image* image, ImageLayout old_layout, ImageLayout new_layout,
	                                          AccessFlags src_access_flags, AccessFlags dst_access_flags, PipelineStageFlags src_stage_flags, PipelineStageFlags dst_stage_flags)
	{
		VkImageAspectFlags aspectMask = IsDepthFormat(static_cast<VulkanImage const*>(image)->GetFormat()) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

		const VkImageMemoryBarrier barrier {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = ConvertToVkAccessFlags(src_access_flags),
			.dstAccessMask = ConvertToVkAccessFlags(dst_access_flags),
			.oldLayout = ConvertToVkImageLayout(old_layout),
			.newLayout = ConvertToVkImageLayout(new_layout),
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = static_cast<const VulkanImage*>(image)->GetNativeHandle(),
			.subresourceRange = {
				.aspectMask = aspectMask,
				.baseMipLevel = 0,
				.levelCount = VK_REMAINING_MIP_LEVELS,
				.baseArrayLayer = 0,
				.layerCount = VK_REMAINING_ARRAY_LAYERS,
			},
		};

		vkCmdPipelineBarrier(m_commandBuffer, ConvertToVkPipelineStageFlags(src_stage_flags), ConvertToVkPipelineStageFlags(dst_stage_flags),
			0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	void VulkanCommandBuffer::CopyBuffer(const Buffer* dst, const Buffer* src)
	{
		const VulkanBuffer* vkdst = static_cast<const VulkanBuffer*>(dst);
		const VulkanBuffer* vksrc = static_cast<const VulkanBuffer*>(src);

		const VkBufferCopy copyRegion {
			.srcOffset = 0,
			.dstOffset = 0,
			.size = min(vkdst->GetSize(), vksrc->GetSize())
		};

		vkCmdCopyBuffer(m_commandBuffer, vksrc->GetNativeHandle(), vkdst->GetNativeHandle(), 1, &copyRegion);
	}

	void VulkanCommandBuffer::CopyBufferToImage(const Image* dst, const Buffer* src, ImageLayout layout)
	{
		const VulkanImage* vkdst = static_cast<const VulkanImage*>(dst);
		const VulkanBuffer* vksrc = static_cast<const VulkanBuffer*>(src);

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

	// Vulkan Buffer
	VulkanBuffer::~VulkanBuffer()
	{
		m_device->DestroyBuffer(this);
	}

	VulkanImage::VulkanImage(VulkanDevice const* device, VkImage image, VkImageView view, VkExtent3D extent, VkFormat format)
	: m_device(device), m_image(image), m_allocation(nullptr), m_allocationInfo(), m_view(apex_new(VulkanImageView)(view, this)), m_extent(extent), m_format(format)
	{
	}

	// Vulkan Image
	VulkanImage::VulkanImage(VulkanDevice const* device, VkImage image, VkImageCreateInfo const& create_info, VkImageView view)
	: m_device(device), m_image(image), m_allocation(nullptr), m_allocationInfo(), m_view(apex_new(VulkanImageView)(view, this)), m_extent(create_info.extent), m_format(create_info.format)
	{
	}

	VulkanImage::VulkanImage(VulkanDevice const* device, VkImage image, VkImageCreateInfo const& create_info, VmaAllocation allocation, VmaAllocationInfo const& allocation_info, VkImageView view)
	: m_device(device), m_image(image), m_allocation(allocation), m_allocationInfo(allocation_info), m_view(apex_new(VulkanImageView)(view, this)), m_extent(create_info.extent), m_format(create_info.format)
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

	VulkanShaderModule::~VulkanShaderModule()
	{
		m_device->DestroyShaderModule(this);
	}

	VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
	{
		m_device->DestroyPipeline(this);
	}

	VulkanComputePipeline::~VulkanComputePipeline()
	{
		m_device->DestroyPipeline(this);
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
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;

		const VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo {
			.sType				= VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		    .messageSeverity	= messageSeverityFlags,
		    .messageType		= messageTypeFlags,
		    .pfnUserCallback	= DebugMessengerCallback,
		    .pUserData			= this,
		};

		const VkInstanceCreateInfo instanceCreateInfo {
			.sType				     = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext				     = &debugUtilsMessengerCreateInfo,
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

		vk::CreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
		if (nullptr != vk::CreateDebugUtilsMessengerEXT)
		{
			axVerifyFmt(VK_SUCCESS == vk::CreateDebugUtilsMessengerEXT(m_instance, &debugUtilsMessengerCreateInfo, VK_NULL_HANDLE, &m_debugUtilsMessenger),
				"Failed to create debug utils messenger!"
			);
		}
		vk::DestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");

		vk::SetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT) vkGetInstanceProcAddr(m_instance, "vkSetDebugUtilsObjectNameEXT");
		if (nullptr == vk::SetDebugUtilsObjectNameEXT)
		{
			axWarnFmt("Could not load function `vkSetDebugUtilsObjectNameEXT`");
			vk::SetDebugUtilsObjectNameEXT = +[](VkDevice, const VkDebugUtilsObjectNameInfoEXT*) { return VK_ERROR_NOT_PERMITTED_EXT; };
		}

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

		VkPhysicalDevice physicalDevice = SelectPhysicalDevice(requiredDeviceFeatures);

		m_device = apex::make_unique<VulkanDevice>(*this, physicalDevice, requiredDeviceFeatures);

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
		if (nullptr != vk::DestroyDebugUtilsMessengerEXT)
			vk::DestroyDebugUtilsMessengerEXT(m_instance, m_debugUtilsMessenger, VK_NULL_HANDLE);
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

	bool VulkanContextImpl::ResizeWindow(u32 width, u32 height) const
	{
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->m_physicalDevice, m_surface, &surfaceCapabilities);

		if (surfaceCapabilities.currentExtent.width == m_device->m_swapchain.extent.width &&
			surfaceCapabilities.currentExtent.height == m_device->m_swapchain.extent.height)
		{
			return false;
		}

		vkDeviceWaitIdle(m_device->m_logicalDevice);

		m_device->CreateSwapchain(m_surface, width, height);

		return true;
	}
#endif // APEX_PLATFORM_WIN32

}

