#pragma once

#include <atomic>
#include <vulkan/vulkan.hpp>

// ApexCore includes
#include "Core/Types.h"
#include "Core/Utility.h"

// ApexGraphics includes
#include "Factory.h"
#include "Containers/AxArray.h"

namespace apex {
namespace gfx {

	// Forward Declarations
	class Device;
	class CommandBuffer;

	struct DeviceProperties;
	struct DeviceFeatures;

	class Buffer;
	class Image;

	class BufferView;
	class ImageView;

	class DescriptorSet;

	class ShaderModule;
	class Pipeline;
	class GraphicsPipeline;
	class ComputePipeline;

	enum class ContextApi
	{
		None,
		Vulkan,
		D3D12,
		WebGpu,
	};

	class ContextBase
	{
	public:
		virtual ~ContextBase() = default;
		
#if APEX_PLATFORM_WIN32
		virtual void Init(HINSTANCE hinstance, HWND hwnd) = 0;
#else
		virtual void Init() = 0;
#endif
		virtual void Shutdown() = 0;

		virtual Device* GetDevice() const = 0;
		virtual void GetDeviceFeatures(DeviceFeatures& UNUSED_ARG(device_features)) const = 0;
		virtual void GetDeviceProperties(DeviceProperties& UNUSED_ARG(device_properties)) const = 0;
		
		virtual void ResizeWindow(u32 width, u32 height) const = 0;
	};

	class Context
	{
	private:
		Context(ContextBase* context) : m_instance(context) {}
	public:
		~Context() { if (m_instance) Shutdown(); }

		static Context CreateContext(ContextApi api);

#if APEX_PLATFORM_WIN32
		void Init(HINSTANCE hinstance, HWND hwnd) { m_instance->Init(hinstance, hwnd); }
#else
		void Init() { m_instance->Init(); }
#endif
		void Shutdown()
		{
			m_instance->Shutdown();
			delete m_instance;
			m_instance = nullptr;
		}
		
		Device* GetDevice() const { return m_instance->GetDevice(); }
		void GetDeviceFeatures(DeviceFeatures& device_features) const { m_instance->GetDeviceFeatures(device_features); }
		void GetDeviceProperties(DeviceProperties& device_properties) const { m_instance->GetDeviceProperties(device_properties); }

		void ResizeWindow(u32 width, u32 height) const { m_instance->ResizeWindow(width, height); }

	private:
		ContextBase* m_instance;
	};

	struct DeviceQueue
	{
		enum Value {
			Graphics,
			Compute,
			Transfer,
		};

		Value value;

		constexpr DeviceQueue(Value val) : value(val) {}

		constexpr operator Value() const { return value; }
		constexpr bool operator == (Value val) const { return value == val; }
	};

	class Device
	{
	public:
		virtual ~Device() = default;

		virtual u32 GetFramesInFlight() const = 0;
		virtual u32 GetCurrentFrameIndex() const = 0;
		virtual Dim2D GetSurfaceDim() const = 0;

		virtual CommandBuffer* AllocateCommandBuffer(u32 queueIdx, u32 frame_index, u32 thread_idx) const = 0;
		virtual void ResetCommandBuffers(u32 thread_idx) const = 0;
		virtual void ResetCommandBuffers() const = 0;
		virtual void SubmitCommandBuffer(DeviceQueue queue, CommandBuffer* command_buffer) const = 0;
		virtual void SubmitImmediateCommandBuffer(DeviceQueue queue, CommandBuffer* command_buffer) const = 0;

		virtual const Image* AcquireNextImage() = 0;
		virtual void Present(DeviceQueue queue) = 0;
		virtual void WaitForQueueIdle(DeviceQueue queue) const = 0;
		virtual void WaitForIdle() const = 0;

		virtual AxArray<DescriptorSet> AllocateDescriptorSets(GraphicsPipeline* pipeline) const = 0;
		virtual void UpdateDescriptorSet(DescriptorSet const& descriptor_set) const = 0;

		virtual ShaderModule* CreateShaderModule(const char* name, const char* filepath) const = 0;
		virtual GraphicsPipeline* CreateGraphicsPipeline(const char* name, GraphicsPipelineCreateDesc const& desc) const = 0;

		virtual Buffer* CreateBuffer(const char* name, BufferCreateDesc const& desc) = 0;
		virtual Buffer* CreateVertexBuffer(const char* name, size_t size, const void* initial_data) = 0;
		virtual Buffer* CreateIndexBuffer(const char* name, size_t size, const void* initial_data) = 0;
		virtual Buffer* CreateStagingBuffer(const char* name, size_t size) = 0;

		virtual Image* CreateImage(const char* name, ImageCreateDesc const& desc) const = 0;
		virtual ImageView* CreateImageView(const char* name, ImageViewCreateDesc const& desc) const = 0;
	};

	class CommandBuffer
	{
	public:
		CommandBuffer() = default;
		virtual ~CommandBuffer() = default;

		virtual void* GetNativeHandle() = 0;

		virtual void Begin() = 0;
		virtual void End() = 0;

		// Graphics Commands
		virtual void BeginRendering(const ImageView* color_image_view, ImageView const* depth_stencil_image_view) = 0;
		virtual void EndRendering() = 0;
		virtual void BindGraphicsPipeline(GraphicsPipeline const* pipeline) = 0;
		virtual void BindDescriptorSet(DescriptorSet const& descriptor_set, GraphicsPipeline const* pipeline) = 0;
		virtual void BindDescriptorSet(DescriptorSet const* descriptor_set, ComputePipeline const* pipeline) = 0;
		virtual void SetViewport(Viewport viewport) = 0;
		virtual void SetScissor(Rect2D scissor) = 0;
		virtual void Clear() = 0;
		virtual void BindVertexBuffer(Buffer const* buffer) = 0;
		virtual void BindIndexBuffer(Buffer const* buffer) = 0;
		virtual void Draw(u32 vertex_count) = 0;
		virtual void DrawIndexed(u32 index_count) = 0;
		virtual void DrawIndirect() = 0;
		virtual void TransitionImage(const Image* image, ImageLayout old_layout, ImageLayout new_layout,
			AccessFlags src_access_flags, AccessFlags dst_access_flags,
			PipelineStageFlags src_stage_flags, PipelineStageFlags dst_stage_flags) = 0;
		virtual void Barrier() = 0;

		// Compute Commands
		virtual void BeginComputePass() = 0;
		virtual void EndComputePass() = 0;
		virtual void BindComputePipeline(ComputePipeline const* pipeline) = 0;
		virtual void Dispatch() = 0;

		// Transfer Commands
		virtual void CopyBuffer(Buffer const* dst, Buffer const* src) = 0;
		virtual void CopyImageToBuffer(Buffer const* dst, Image const* src) = 0;
		virtual void CopyImage(Image const* dst, Image const* src) = 0;
		virtual void CopyBufferToImage(const Image* dst, const Buffer* src, ImageLayout layout) = 0;
		virtual void CopyQueryResults() = 0;
	};

	// Resources
	class Resource
	{
	public:
		Resource() = default;
		virtual ~Resource() = default;

		Resource(Resource const&) = delete;
		Resource& operator=(Resource const&) = delete;

		virtual bool IsValid() const = 0;
		bool IsImage() const { return m_isImage; }
		bool IsBuffer() const { return !m_isImage; }
		bool IsView() const { return m_isView; }

		void AddRef() { (void)m_refCount++; }
		void Release() { if (--m_refCount == 0) delete this; }

	protected:
		std::atomic_uint32_t m_refCount = 1;

		// some metadata about handle - is it a view, is it a buffer or texture, is it a sampler, etc.
		u16 m_isImage : 1;
		u16 m_isView : 1;
	};

	class Buffer : public Resource
	{
	public:
		Buffer()
		{
			m_isImage = false;
			m_isView = false;
		}

		virtual ~Buffer() override = default;

		virtual void* GetMappedPointer() const = 0;
	};

	class Image : public Resource
	{
	public:
		Image()
		{
			m_isImage = true;
			m_isView = false;
		}

		virtual ~Image() override = default;

		virtual bool HasView() const = 0;
		virtual const ImageView* GetView() const = 0;
		virtual ImageView* GetView() = 0;
	};

	class BufferView : public Resource
	{
	};

	class ImageView : public Resource
	{
	public:
		virtual const Image* GetOwner() const = 0;
		virtual Image* GetOwner() = 0;
	};

	// Descriptors
	enum class DescriptorType
	{
		eInputAttachment,
		eSampler,
		eCombinedImageSampler,
		eSampledImage,
		eStorageImage,
		eUniformBuffer,
		eStorageBuffer,

		// HLSL descriptor types
		eShaderResourceView = eStorageImage,
		eConstantBufferView = eUniformBuffer,
		eUnorderedAccessView = eStorageBuffer,
		eTexture = eSampledImage,
	};

	class Descriptor
	{
	public:
		Descriptor(DescriptorType type, Resource* resource) : m_type(type), m_resource(resource) {}

		DescriptorType GetType() const { return m_type; }
		Resource* GetResource() { return m_resource; }
		Resource const* GetResource() const { return m_resource; }

	private:
		DescriptorType m_type;
		Resource* m_resource;
	};
	/*class ShaderAttachment : public Descriptor {};
	class ColorAttachment : public ShaderAttachment {};
	class DepthStencilAttachment : public ShaderAttachment {};
	class SampledImage : public Descriptor {};
	class StorageImage : public Descriptor {};
	class UniformBuffer : public Descriptor {};
	class StorageBuffer : public Descriptor {};*/

	class DescriptorSet
	{
	public:
		constexpr DescriptorSet(void* handle) : m_handle(handle) {}

		void* GetNativeHandle() const { return m_handle; }

		void Add(Descriptor const& descriptor)
		{
			if (m_descriptors.capacity() == 0)
				m_descriptors.reserve(2);

			if (m_descriptors.capacity() < m_descriptors.size() + 1)
				m_descriptors.reserve(m_descriptors.capacity() * 2);

			m_descriptors.append(descriptor);
		}
		void Clear() { m_descriptors.clear(); }

		AxArray<Descriptor> const& GetDescriptors() const { return m_descriptors; }

	private:
		void* m_handle {};

		AxArray<Descriptor> m_descriptors;
	};


	// Pipeline
	class ShaderModule
	{
	public:
		virtual ~ShaderModule() = default;
	};

	class Pipeline
	{
	public:
		virtual ~Pipeline() = default;

		bool IsGraphicsPipeline() const { return !IsComputeEnabled(); }
		bool IsComputePipeline() const { return IsComputeEnabled(); }

	protected:
		virtual bool IsComputeEnabled() const = 0;
	};

	class GraphicsPipeline : public Pipeline
	{
	private:
		bool IsComputeEnabled() const override { return false; }
	};

	class ComputePipeline : public Pipeline
	{
	private:
		bool IsComputeEnabled() const override { return true; }
	};


	// Rendering
	class RenderPass; // collection of input and output states, bindings. -> declarative definition of state transitions
	class Technique; // sequence of render passes, pipelines, their resources, and the rendering algorithm
	class Effect; // interface to be implemented by any shader effect or renderer


	// Task Graph (FrameGraph)
	class TaskGraph;
	class TaskDesc;
	class TaskAttachmentDesc;


	// Reference counted resource reference
	template <typename ResourceT>
	class ResourceRef
	{
	public:
		using value_type = ResourceT;
		using pointer    = ResourceT*;
		using reference  = ResourceT&;

		ResourceRef() = default;
		ResourceRef(std::nullptr_t) {}
		ResourceRef(pointer resource) : m_ptr(resource) {}

		ResourceRef(ResourceRef const& other) : m_ptr(other.m_ptr) { if (m_ptr) m_ptr->AddRef(); }
		ResourceRef& operator=(ResourceRef const& other)
		{
			if (this == &other)
				return *this;
			if (m_ptr)
				m_ptr->Release();
			m_ptr = other.m_ptr;
			m_ptr->AddRef();
			return *this;
		}

		ResourceRef(ResourceRef&& other) noexcept : m_ptr(other.m_ptr) { other.m_ptr = nullptr; }
		ResourceRef& operator=(ResourceRef&& other) noexcept
		{
			if (this == &other)
				return *this;
			if (m_ptr)
				m_ptr->Release();
			m_ptr = other.m_ptr;
			other.m_ptr = nullptr;
			return *this;
		}

		ResourceRef& operator=(std::nullptr_t) { Reset(); return *this; }

		~ResourceRef() { if (m_ptr) m_ptr->Release(); }

		reference Get() { return *m_ptr; }
		pointer GetPointer() { return m_ptr; }

		void Reset()
		{
			if (m_ptr) m_ptr->Release();
			m_ptr = nullptr;
		}

		pointer operator->() { return m_ptr; }

	private:
		ResourceT* m_ptr = nullptr;
	};

	template <typename R> using Ref = ResourceRef<R>;

	using BufferRef = ResourceRef<Buffer>;
	using ImageRef = ResourceRef<Image>;


} // namespace gfx
} // namespace apex
