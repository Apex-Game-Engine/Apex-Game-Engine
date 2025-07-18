#pragma once

#include <atomic>

// ApexCore includes
#include "Core/Types.h"
#include "Core/Utility.h"

// ApexGraphics includes
#include "Factory.h"
#include "Containers/AxArray.h"
#include "Math/Vector4.h"

namespace apex::plat
{
	class PlatformWindow;
}

namespace apex {
namespace gfx {

	// Forward Declarations
	class Device;
	class Queue;
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

	class Fence;

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
		
		virtual void Init(const plat::PlatformWindow& window) = 0;
		virtual void Shutdown() = 0;

		virtual ContextApi GetApi() const = 0;
		virtual Device* GetDevice() const = 0;
		virtual void GetDeviceFeatures(DeviceFeatures& device_features) const = 0;
		virtual void GetDeviceProperties(DeviceProperties& device_properties) const = 0;
		
		virtual void ResizeSurface(u32 width, u32 height) const = 0;
	};

	class Context
	{
	private:
		Context(ContextBase* context) : m_base(context) {}
	public:
		constexpr Context() = default;
		~Context() = default;

		static Context CreateContext(ContextApi api);

		void Init(const plat::PlatformWindow& window) { m_base->Init(window); }
		void Shutdown()
		{
			m_base->Shutdown();
			delete m_base;
			m_base = nullptr;
		}

		ContextBase* GetBase() const { return m_base; }

		ContextApi GetApi() const { return m_base->GetApi(); }
		Device* GetDevice() const { return m_base->GetDevice(); }
		void GetDeviceFeatures(DeviceFeatures& device_features) const { m_base->GetDeviceFeatures(device_features); }
		void GetDeviceProperties(DeviceProperties& device_properties) const { m_base->GetDeviceProperties(device_properties); }

		void ResizeWindow(u32 width, u32 height) const { m_base->ResizeSurface(width, height); }

	private:
		ContextBase* m_base;
	};

	class Device
	{
	public:
		virtual ~Device() = default;

		virtual u32 GetTotalFramesPresented() const = 0;
		virtual u32 GetFramesInFlight() const = 0;
		virtual u32 GetCurrentFrameIndex() const = 0;
		virtual u32 GetCurrentSwapchainImageIndex() const = 0;
		virtual Dim2D GetSurfaceDim() const = 0;

		virtual Queue* GetQueue(QueueType queue_type) = 0;
		virtual const Queue* GetQueue(QueueType queue_type) const = 0;

		virtual const Image* AcquireNextImage() = 0;
		virtual void WaitForIdle() const = 0;

		virtual CommandBuffer* AllocateCommandBuffer(QueueType queue, u32 frame, u32 thread) const = 0;

		virtual AxArray<DescriptorSet> AllocateDescriptorSets(GraphicsPipeline* pipeline) const = 0;
		virtual void UpdateDescriptorSet(DescriptorSet const& descriptor_set) const = 0;
		virtual void BindSampledImage(ImageView* image_view) = 0;
		virtual void BindStorageImage(ImageView* image_view) = 0;
		virtual void BindUniformBuffer(Buffer* buffer) = 0;
		virtual void BindStorageBuffer(Buffer* buffer) = 0;

		virtual ShaderModule* CreateShaderModule(const char* name, const char* filepath) const = 0;
		virtual GraphicsPipeline* CreateGraphicsPipeline(const char* name, GraphicsPipelineCreateDesc const& desc) const = 0;
		virtual ComputePipeline* CreateComputePipeline(const char* name, ComputePipelineCreateDesc const& desc) const = 0;

		virtual Buffer* CreateBuffer(const char* name, BufferCreateDesc const& desc) = 0;
		virtual Buffer* CreateVertexBuffer(const char* name, size_t size, const void* initial_data) = 0;
		virtual Buffer* CreateIndexBuffer(const char* name, size_t size, const void* initial_data) = 0;
		virtual Buffer* CreateStagingBuffer(const char* name, size_t size) = 0;

		virtual Image* CreateImage(const char* name, ImageCreateDesc const& desc) = 0;
		virtual ImageView* CreateImageView(const char* name, ImageViewCreateDesc const& desc) const = 0;

		virtual Fence* CreateFence(const char* name, u64 init_value) = 0;
	};

	class Queue
	{
	public:
		virtual ~Queue() = default;

		virtual void ResetCommandBuffers(u32 frame_index, u32 thread_idx) const = 0;
		virtual void ResetCurrentFrameCommandBuffers() const = 0;

		virtual void Submit(CommandBuffer* command_buffer) = 0;
		virtual void Submit(CommandBuffer* command_buffer, bool wait_image_acquired, PipelineStageFlags wait_stage_mask, bool signal_render_complete) = 0;
		virtual void Submit(CommandBuffer* command_buffer, Fence* fence, u64 wait_value, PipelineStageFlags wait_stage_mask, u64 signal_value) = 0;
		virtual void Submit(QueueSubmitDesc const& desc) = 0;
		virtual void Flush() = 0;
		virtual void Present() = 0;
		virtual void WaitForIdle() = 0;

		virtual QueueType GetType() const = 0;
		virtual bool CanPresent() const = 0;
	};

	class CommandBuffer
	{
	public:
		CommandBuffer() = default;
		virtual ~CommandBuffer() = default;

		virtual void* GetNativeHandle() = 0;

		virtual void Begin() = 0;
		virtual void End() = 0;

		virtual void BindGlobalDescriptorSets() = 0;

		// Compute Commands
		virtual void BindComputePipeline(ComputePipeline const* pipeline) = 0;
		virtual void Dispatch(Dim3D group_counts) = 0;

		// Graphics Commands
		virtual void BeginRendering(const ImageView* color_image_view, ImageView const* depth_stencil_image_view, bool clear = true) = 0;
		virtual void EndRendering() = 0;
		virtual void BindGraphicsPipeline(GraphicsPipeline const* pipeline) = 0;
		virtual void BindDescriptorSet(DescriptorSet const& descriptor_set, GraphicsPipeline const* pipeline) = 0;
		virtual void BindDescriptorSet(DescriptorSet const* descriptor_set, ComputePipeline const* pipeline) = 0;
		virtual void PushConstants(AxArrayRef<const char> const& bytes) = 0;
		virtual void SetViewport(Viewport viewport) = 0;
		virtual void SetScissor(Rect2D scissor) = 0;
		virtual void Clear() = 0;
		virtual void BindVertexBuffer(Buffer const* buffer) = 0;
		virtual void BindIndexBuffer(Buffer const* buffer) = 0;
		virtual void Draw(u32 vertex_count) = 0;
		virtual void DrawIndexed(u32 index_count) = 0;
		virtual void DrawIndirect() = 0;
		virtual void TransitionImage(const Image* image,
									 ImageLayout old_layout, PipelineStageFlags src_stage_mask, AccessFlags src_access_mask, QueueType src_queue,
									 ImageLayout new_layout, PipelineStageFlags dst_stage_mask, AccessFlags dst_access_mask, QueueType dst_queue) = 0;
		virtual void Barrier() = 0;
		virtual void InsertMemoryBarrier(PipelineStageFlags src_stage_mask, AccessFlags src_access_mask,
		                                 PipelineStageFlags dst_stage_mask, AccessFlags dst_access_mask) = 0;
		//virtual void InsertExecutionBarrier() = 0;

		// Transfer Commands
		virtual void BlitImage(Image const* src, ImageLayout src_layout, Image const* dst, ImageLayout dst_layout) = 0;
		virtual void CopyBuffer(Buffer const* src, Buffer const* dst) = 0;
		virtual void CopyImageToBuffer(Image const* src, Buffer const* dst) = 0;
		virtual void CopyImage(Image const* src, Image const* dst) = 0;
		virtual void CopyBufferToImage(Buffer const* src, Image const* dst, ImageLayout layout) = 0;
		virtual void CopyQueryResults() = 0;

		// Marker and Label Commands
		virtual void PushLabel(const char* label_str, math::Vector4 const& color = {}) = 0;
		virtual void PopLabel() = 0;

		template <typename T>
		void PushConstants(T const& data)
		{
			PushConstants({ reinterpret_cast<const char*>(&data), sizeof(T) });
		}
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
		virtual size_t GetSize() const = 0;
		virtual u32 GetBindlessIndex(BindlessDescriptorType descriptor_type) const = 0;
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

		virtual u32 GetBindlessIndex(BindlessDescriptorType descriptor_type) const = 0;
	};

	// Descriptors
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

	// Synchronization
	class Fence
	{
	public:
		static constexpr u64 InvalidValue = -1;

		virtual ~Fence() = default;

		virtual void Signal(u64 value) = 0;
		virtual void Wait(u64 value) = 0;
		virtual u64 GetValue() const = 0;
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


	class AutoSubmitCommandBuffer
	{
	public:
		AutoSubmitCommandBuffer(Context& ctx, QueueType queue_type)
			: m_queue(ctx.GetDevice()->GetQueue(queue_type)), m_commandBuffer(ctx.GetDevice()->AllocateCommandBuffer(queue_type, 0, 0))
		{}

		~AutoSubmitCommandBuffer()
		{
			m_queue->Submit(m_commandBuffer);
			m_queue->WaitForIdle();
			delete m_commandBuffer;
		}

		Queue* GetQueue() const { return m_queue; }
		CommandBuffer* GetCommandBuffer() const { return m_commandBuffer; }

		operator bool() const { return m_commandBuffer && m_queue; }

	private:
		Queue* m_queue;
		CommandBuffer* m_commandBuffer;
	};

	class ScopedCommandBufferLabel
	{
	public:
		ScopedCommandBufferLabel(CommandBuffer* command_buffer, const char* label_str, math::Vector4 const& color = {})
			: m_commandBuffer(command_buffer)
		{
			m_commandBuffer->PushLabel(label_str, color);
		}

		~ScopedCommandBufferLabel()
		{
			m_commandBuffer->PopLabel();
		}

	private:
		CommandBuffer* m_commandBuffer;
	};

} // namespace gfx
} // namespace apex

#define ScopedGpuLabel(cmdbuf)					apex::gfx::ScopedCommandBufferLabel CONCAT(__scoped_cmdbuf_label_,__LINE__) ((cmdbuf), (__FUNCTION__))
#define ScopedGpuLabelN(cmdbuf, label)			apex::gfx::ScopedCommandBufferLabel CONCAT(__scoped_cmdbuf_label_,__LINE__) ((cmdbuf), (label))
#define ScopedGpuLabelC(cmdbuf, color)			apex::gfx::ScopedCommandBufferLabel CONCAT(__scoped_cmdbuf_label_,__LINE__) ((cmdbuf), (__FUNCTION__), (color))
#define ScopedGpuLabelNC(cmdbuf, label, color)	apex::gfx::ScopedCommandBufferLabel CONCAT(__scoped_cmdbuf_label_,__LINE__) ((cmdbuf), (label), (color))
