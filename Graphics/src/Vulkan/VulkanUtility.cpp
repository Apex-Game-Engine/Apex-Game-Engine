#include "Graphics/Vulkan/VulkanUtility.h"

#include "Core/Types.h"
#include "Containers/AxArray.h"

#include <vulkan/vulkan_core.h>


namespace apex::vk {

	
	bool check_validation_layer_support(const char* validationLayerNames[], size_t validationLayerCount)
	{
		uint32 instanceLayerCount;
		vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);

		AxArray<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
		vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());

		AxArray<bool> validationLayersPresent(instanceLayerCount);
		validationLayersPresent.resize(instanceLayerCount, false);

		size_t numValidationLayersPresent = 0;

		for (VkLayerProperties& layer : instanceLayerProperties)
		{
			for (size_t i = 0; i < validationLayerCount; i++)
			{
				if (!validationLayersPresent[i] && strcmp(layer.layerName, validationLayerNames[i]) == 0)
				{
					validationLayersPresent[i] = true;
					numValidationLayersPresent++;
				}
			}
		}

		return numValidationLayersPresent == validationLayerCount;
	}

}
