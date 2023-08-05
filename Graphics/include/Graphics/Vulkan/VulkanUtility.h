#pragma once

namespace apex {
namespace vk {

	bool check_validation_layer_support(const char* validationLayerNames[], size_t validationLayerCount);

	bool check_instance_extensions_support(const char* instanceExtensionNames[], size_t instanceExtensionCount);

	bool check_device_extensions_support(const char* deviceExtensionNames[], size_t deviceExtensionCount);

}
}
