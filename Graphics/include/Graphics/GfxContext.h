#pragma once
#include "GfxContext.h"
#include "Core/Types.h"

namespace apex {
namespace gfx {

	using GfxHandle = u64;

	struct DeviceProperties
	{
	};

	struct DeviceFeatures
	{
	};

	class GfxContext
	{
	public:
		static void Initialize();
		static void Shutdown();

		void SelectDevice(/* DeviceSelectionParams */);
		void GetDeviceFeatures(DeviceFeatures& device_features);
		void GetDeviceProperties(DeviceProperties& device_properties);
	};

	class GfxResource
	{
	public:
		GfxResource() = default;

		bool IsValid() { return m_handle != kInvalidHandle; }

	private:
		static constexpr GfxHandle kInvalidHandle = ~0;

		GfxHandle m_handle { kInvalidHandle };
		// some metadata about handle - is it a view, is it a buffer or texture, is it a sampler, etc.
	};


	// Resources
	class Buffer : public GfxResource
	{
	};

	class Image : public GfxResource
	{
	};

	class RenderTarget : public Image
	{
	};

	class DepthStencilTarget : public Image
	{
	};

	class Texture : public Image
	{
	};


	// Buffers
	class VertexBuffer : public Buffer
	{
	};

	class IndexBuffer : public Buffer
	{
	};

	class UniformBuffer : public Buffer
	{
	};

	class StorageBuffer : public Buffer
	{
	};


	// Descriptors
	class ResourceDescriptor {};
	class BufferDescriptor : ResourceDescriptor {};
	class ImageDescriptor : ResourceDescriptor {};


	// Views
	class ResourceView {};
	class ImageView : public ResourceView {};
	class UniformBufferView : public ResourceView {};
	class StorageBufferView : public ResourceView {};


	// Shader
	class Shader {};
	class VertexShader {};
	class FragmentShader {};
	class GeometryShader {};
	class ComputeShader {};


	// Pipeline
	class ShaderSet {};
	class PipelineStateObject {};
	class GraphicsPSO {};
	class ComputePSO {};


	// Rendering
	class RenderPass; // collection of input and output states, bindings. -> declarative definition of state transitions
	class Technique; // sequence of render passes, pipelines, their resources, and the rendering algorithm
	class Effect; // interface to be implemented by any shader effect or renderer

}
}
