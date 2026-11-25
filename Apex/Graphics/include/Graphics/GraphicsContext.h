#pragma once

#include <atomic>

// ApexCore includes
#include "Core/Types.h"

// ApexGraphics includes
#include "Factory.h"
#include "Containers/AxArray.h"
#include "Math/Color.h"

namespace apex::plat
{
	class PlatformWindow;
}

#define GFX_USE_BINDLESS_DESCRIPTORS 1

namespace apex {
namespace gfx {
	class VertexBuffer;

	constexpr size_t MAX_FRAMES_IN_FLIGHT = 3;
	constexpr size_t MAX_RENDER_THREADS = 6;
	constexpr size_t MAX_COMMAND_POOLS_PER_QUEUE = MAX_FRAMES_IN_FLIGHT * MAX_RENDER_THREADS;

	enum class Api
	{
		None,
		Vulkan,
		D3D12,
		WebGpu,
	};

	class Context
	{
	public:
		virtual ~Context() = default;

		virtual Api GetApi() const = 0;
		virtual void GetDeviceFeatures(DeviceFeatures& device_features) const = 0;
		virtual void GetDeviceProperties(DeviceProperties& device_properties) const = 0;

		virtual u32 GetTotalFramesPresented() const = 0;
		virtual u32 GetMaxFrameCount() const = 0;
		virtual u32 GetCurrentFrameIndex() const = 0;
		virtual u32 GetCurrentSwapchainImageIndex() const = 0;
		virtual Dim2D GetSurfaceDim() const = 0;

		// Resource creation functions
		virtual ShaderModule* CreateShaderModule(const char* name, const char* filepath) const = 0;
		virtual GraphicsPipeline* CreateGraphicsPipeline(const char* name, GraphicsPipelineCreateDesc const& desc) const = 0;
		virtual ComputePipeline* CreateComputePipeline(const char* name, ComputePipelineCreateDesc const& desc) const = 0;

		virtual Buffer* CreateBuffer(const char* name, BufferCreateDesc const& desc) = 0;
		virtual Image* CreateImage(const char* name, ImageCreateDesc const& desc) = 0;
		virtual ImageView* CreateImageView(const char* name, ImageViewCreateDesc const& desc) const = 0;

		virtual Fence* CreateFence(const char* name, u64 init_value) = 0;
		
		// virtual CommandBuffer* AllocateTransferCommandBuffer() const = 0;

		// Resource binding functions
	#if !GFX_USE_BINDLESS_DESCRIPTORS
		virtual AxArray<DescriptorSet> AllocateDescriptorSets(GraphicsPipeline* pipeline) const = 0;
		virtual void UpdateDescriptorSet(DescriptorSet const& descriptor_set) const = 0;
	#else
		virtual void BindSampledImage(ImageView* image_view) = 0;
		virtual void BindStorageImage(ImageView* image_view) = 0;
		virtual void BindUniformBuffer(Buffer* buffer, u64 offset = 0, u64 size = WHOLE_SIZE) = 0;
		virtual void BindStorageBuffer(Buffer* buffer, u64 offset = 0, u64 size = WHOLE_SIZE, u32 index = INVALID_INDEX) = 0;
	#endif // !GFX_USE_BINDLESS_DESCRIPTORS

		virtual const Image* AcquireNextImage() = 0;
		virtual void ResizeSurface(u32 width, u32 height) const = 0;
		virtual void Submit(CommandBuffer* command_buffer) = 0;
		virtual void Submit(CommandBuffer* command_buffer, bool wait_image_acquired, PipelineStageFlags wait_stage_mask, bool signal_render_complete) = 0;
		virtual void Submit(CommandBuffer* command_buffer, Fence* fence, u64 wait_value, PipelineStageFlags wait_stage_mask, u64 signal_value) = 0;
		virtual void Submit(QueueSubmitDesc const& desc) = 0;
		virtual void Flush() = 0;
		virtual bool Present() = 0;
		virtual void WaitForIdle() = 0;

	protected:
		virtual void Init(const plat::PlatformWindow& window) = 0;
		virtual void Shutdown() = 0;
		friend struct Factory;
	};

	/*class Queue
	{
	public:
		virtual ~Queue() = default;

		virtual CommandBuffer* AllocateCommandBuffer(u32 frame_index = -1) = 0;
		virtual void ResetCommandBuffers(u32 frame_index = -1) = 0;

		virtual void Submit(CommandBuffer* command_buffer) = 0;
		virtual void Submit(CommandBuffer* command_buffer, bool wait_image_acquired, PipelineStageFlags wait_stage_mask, bool signal_render_complete) = 0;
		virtual void Submit(CommandBuffer* command_buffer, Fence* fence, u64 wait_value, PipelineStageFlags wait_stage_mask, u64 signal_value) = 0;
		virtual void Submit(QueueSubmitDesc const& desc) = 0;
		virtual void Flush() = 0;
		virtual bool Present() = 0;
		virtual void WaitForIdle() = 0;

		virtual QueueType GetType() const = 0;
		virtual bool CanPresent() const = 0;
	};*/

	class CommandBuffer
	{
	public:
		virtual ~CommandBuffer() = default;

		virtual void* GetNativeHandle() = 0;
		virtual QueueType GetQueueType() const = 0;

		virtual void Begin() = 0;
		virtual void End() = 0;

		virtual void BindGlobalDescriptorSets() = 0;

		// Compute Commands
		virtual void BindComputePipeline(ComputePipeline const* pipeline) = 0;
		virtual void Dispatch(Dim3D group_counts) = 0;

		// Graphics Commands
		virtual void BeginRendering(ImageView const* color_image_view, ImageView const* depth_stencil_image_view, LoadOp load_op = LoadOp::DontCare) = 0;
		virtual void BeginRendering(AxSpan<ImageView const*> color_image_views, ImageView const* depth_stencil_image_view, LoadOp load_op = LoadOp::DontCare) = 0;
		virtual void EndRendering() = 0;
		virtual void BindGraphicsPipeline(GraphicsPipeline const* pipeline) = 0;
		virtual void BindDescriptorSet(DescriptorSet const& descriptor_set, GraphicsPipeline const* pipeline) = 0;
		virtual void BindDescriptorSet(DescriptorSet const* descriptor_set, ComputePipeline const* pipeline) = 0;
		virtual void PushConstants(AxSpan<const char> const& bytes) = 0;
		virtual void SetViewport(Viewport viewport) = 0;
		virtual void SetScissor(Rect2D scissor) = 0;
		virtual void SetClearColor(Color4f rgba) = 0;
		virtual void Clear() = 0;
		virtual void BindVertexBuffer(VertexBuffer const* buffer) = 0;
		virtual void BindIndexBuffer(Buffer const* buffer) = 0;
		virtual void Draw(u32 vertex_count) = 0;
		virtual void DrawIndexed(u32 index_count) = 0;
		virtual void DrawIndirect() = 0;
		virtual void TransitionImageCrossQueueFrom(const Image* image,
									 ImageLayout old_layout, PipelineStageFlags src_stage_mask, AccessFlags src_access_mask, QueueType src_queue,
									 ImageLayout new_layout, PipelineStageFlags dst_stage_mask, AccessFlags dst_access_mask) = 0;
		virtual void TransitionImageCrossQueueTo(const Image* image,
									 ImageLayout old_layout, PipelineStageFlags src_stage_mask, AccessFlags src_access_mask,
									 ImageLayout new_layout, PipelineStageFlags dst_stage_mask, AccessFlags dst_access_mask, QueueType dst_queue) = 0;
		virtual void TransitionImage(const Image* image,
									 ImageLayout old_layout, PipelineStageFlags src_stage_mask, AccessFlags src_access_mask,
									 ImageLayout new_layout, PipelineStageFlags dst_stage_mask, AccessFlags dst_access_mask) = 0;
		virtual void TransitionImage(const Image* image, ImageLayout old_layout, ImageLayout new_layout) = 0;
		virtual void Barrier() = 0;
		virtual void InsertMemoryBarrier(PipelineStageFlags src_stage_mask, AccessFlags src_access_mask,
		                                 PipelineStageFlags dst_stage_mask, AccessFlags dst_access_mask) = 0;
		//virtual void InsertExecutionBarrier() = 0;

		// Transfer Commands
		virtual void BlitImage(Image const* src, ImageLayout src_layout, Image const* dst, ImageLayout dst_layout) = 0;
		virtual void CopyBuffer(Buffer const* src, Buffer const* dst) = 0;
		virtual void CopyImageToBuffer(Image const* src, Buffer const* dst) = 0;
		virtual void CopyImage(Image const* src, ImageLayout src_layout, Image const* dst, ImageLayout dst_layout) = 0;
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
	enum class ResourceType : u8
	{
		Buffer	 = 0,
		Image	 = 1,
	};

	class Unknown
	{
	public:
		Unknown() = default;
		virtual ~Unknown() = 0;
	protected:
		void AddRef() { (void)m_refCount++; }
		void Release() { if (--m_refCount == 0) delete this; }	
	private:
		std::atomic_uint32_t m_refCount = 1;

		template <typename> friend class RefPtr;
		friend class CleanupQueue;
	};

	class Resource : public Unknown
	{
	protected:
		Resource(const char* DEBUG_ONLY(name), ResourceType type, bool is_view)
			: m_type((u8)type), m_isView(is_view) DEBUG_ONLY(, m_debugName(name))
		{
		}
	public:
		~Resource() override = default;

		Resource(Resource const&) = delete;
		Resource& operator=(Resource const&) = delete;

		virtual bool IsValid() const { return false; }
		DEBUG_ONLY(const char* GetName() const { return m_debugName; })
		u8 GetType() const { return m_type; }
		bool IsBuffer() const { return m_type == (u8)ResourceType::Buffer; }
		bool IsImage() const { return m_type == (u8)ResourceType::Image; }
		bool IsView() const { return m_isView; }

	private:
		// some metadata about handle - is it a view, is it a buffer or texture, is it a sampler, etc.
		u16 m_type : 2; // ResourceType
		u16 m_isView : 1;

		DEBUG_ONLY(const char* m_debugName;)
	};

	class Buffer : public Resource
	{
	public:
		Buffer(const char* name)
			: Resource(name, ResourceType::Buffer, false)
		{
		}

		~Buffer() override = default;

		virtual void* GetMappedPointer() const = 0;
		virtual size_t GetSize() const = 0;
		virtual GlobalDescriptorBinding GetGlobalDescriptorBinding() const = 0;
		virtual u16 GetGlobalDescriptorIndex() const = 0;
	};

	class VertexBuffer
	{
	public:
		VertexBuffer(Buffer* buffer, VertexFormat format, u32 vertex_count)
			: m_buffer(buffer), m_vertexFormat(std::move(format)), m_vertexCount(vertex_count)
		{
		}

		void* GetMappedPointer() const { return m_buffer->GetMappedPointer(); }
		size_t GetSize() const { return m_buffer->GetSize(); }
		GlobalDescriptorBinding GetGlobalDescriptorBinding() const { return m_buffer->GetGlobalDescriptorBinding(); }
		u16 GetGlobalDescriptorIndex() const { return m_buffer->GetGlobalDescriptorIndex(); }
		Buffer* GetBuffer() const { return m_buffer; }
		VertexFormat const& GetVertexFormat() const { return m_vertexFormat; }
		u32 GetVertexCount() const { return m_vertexCount; }

	private:
		Buffer* m_buffer;
		VertexFormat m_vertexFormat;
		u32 m_vertexCount;
	};

	class Image : public Resource
	{
	public:
		Image(const char* name)
			: Resource(name, ResourceType::Image, false)
		{
		}

		virtual ~Image() override = default;

		virtual bool HasView() const = 0;
		virtual const ImageView* GetView() const = 0;
		virtual ImageView* GetView() = 0;
	};

	class BufferView : public Resource
	{
	public:
		BufferView(const char* name)
			: Resource(name, ResourceType::Buffer, true)
		{
		}
	};

	class ImageView : public Resource
	{
	public:
		ImageView(const char* name)
			: Resource(name, ResourceType::Image, true)
		{
		}

		virtual const Image* GetOwner() const = 0;
		virtual Image* GetOwner() = 0;
		
		virtual GlobalDescriptorBinding GetGlobalDescriptorBinding() const = 0;
		virtual u16 GetGlobalDescriptorIndex() const = 0;
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
	class ShaderModule : public Unknown
	{
	public:
		ShaderModule(const char* name)
		{
		}
		~ShaderModule() override = default;
	};

	class Pipeline : public Unknown
	{
	public:
		Pipeline(const char* name)
		{
		}
		~Pipeline() override = default;

		bool IsGraphicsPipeline() const { return !IsComputeEnabled(); }
		bool IsComputePipeline() const { return IsComputeEnabled(); }

	protected:
		virtual bool IsComputeEnabled() const = 0;
	};

	class GraphicsPipeline : public Pipeline
	{
	public:
		GraphicsPipeline(const char* name) : Pipeline(name) {}
		~GraphicsPipeline() override = default;

		virtual u64 GetVertexFormatHash() const = 0;
	private:
		bool IsComputeEnabled() const override { return false; }
	};

	class ComputePipeline : public Pipeline
	{
	public:
		ComputePipeline(const char* name) : Pipeline(name) {}
		~ComputePipeline() override = default;
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

	// Reference counted resource reference
	template <typename _Unknown>
	class RefPtr
	{
	public:
		using value_type = _Unknown;
		using pointer    = _Unknown*;
		using reference  = _Unknown&;

		RefPtr() = default;
		RefPtr(std::nullptr_t) {}
		RefPtr(pointer resource) : m_ptr(resource)
		{
		}

		RefPtr(RefPtr const& other) : m_ptr(other.m_ptr) { if (m_ptr) m_ptr->AddRef(); }
		RefPtr& operator=(RefPtr const& other)
		{
			if (this == &other)
				return *this;
			if (m_ptr)
				m_ptr->Release();
			m_ptr = other.m_ptr;
			m_ptr->AddRef();
			return *this;
		}

		RefPtr(RefPtr&& other) noexcept : m_ptr(other.m_ptr) { other.m_ptr = nullptr; }
		RefPtr& operator=(RefPtr&& other) noexcept
		{
			if (this == &other)
				return *this;
			if (m_ptr)
				m_ptr->Release();
			m_ptr = other.m_ptr;
			other.m_ptr = nullptr;
			return *this;
		}

		RefPtr& operator=(std::nullptr_t) { Reset(); return *this; }

		~RefPtr() { if (m_ptr) m_ptr->Release(); }

		pointer Get() const { return m_ptr; }

		void Reset()
		{
			if (m_ptr) m_ptr->Release();
			m_ptr = nullptr;
		}

		void Swap(RefPtr& other)
		{
			if (this == &other)
				return;
			pointer tmp = other.m_ptr;
			other.m_ptr = m_ptr;
			m_ptr = tmp;
		}

		pointer operator->() const { return m_ptr; }

		operator bool() const { return m_ptr; }

	private:
		_Unknown* m_ptr = nullptr;
	};

	using BufferRef = RefPtr<Buffer>;
	using ImageRef = RefPtr<Image>;
	using ShaderModuleRef = RefPtr<ShaderModule>;
	using GraphicsPipelineRef = RefPtr<GraphicsPipeline>;
	using ComputePipelineRef = RefPtr<ComputePipeline>;

	class AutoSubmitCommandBuffer
	{
	public:
		AutoSubmitCommandBuffer(Context& context, CommandBuffer& command_buffer)
			: m_context(&context), m_commandBuffer(&command_buffer)
		{}

		~AutoSubmitCommandBuffer()
		{
			m_context->Submit(m_commandBuffer);
			m_context->WaitForIdle();
			delete m_commandBuffer;
		}

		CommandBuffer* GetCommandBuffer() const { return m_commandBuffer; }

		operator bool() const { return m_commandBuffer; }

	private:
		Context* m_context;
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

	class CleanupQueue
	{
	public:
		CleanupQueue() = default;
		~CleanupQueue()
		{
			Cleanup();
		}

		void Add(Resource* resource)
		{
			m_resources.emplace_back(resource);
		}

		void Cleanup()
		{
			for (auto& resource : m_resources)
			{
				if (resource)
				{
					resource->Release();
					resource = nullptr;
				}
			}
			m_resources.reset();
		}

	private:
		AxArray<Resource*> m_resources;
	};

} // namespace gfx
} // namespace apex

#define ScopedGpuLabel(cmdbuf)					apex::gfx::ScopedCommandBufferLabel CONCAT(__scoped_cmdbuf_label_,__LINE__) ((cmdbuf), (__FUNCTION__))
#define ScopedGpuLabelN(cmdbuf, label)			apex::gfx::ScopedCommandBufferLabel CONCAT(__scoped_cmdbuf_label_,__LINE__) ((cmdbuf), (label))
#define ScopedGpuLabelC(cmdbuf, color)			apex::gfx::ScopedCommandBufferLabel CONCAT(__scoped_cmdbuf_label_,__LINE__) ((cmdbuf), (__FUNCTION__), (color))
#define ScopedGpuLabelNC(cmdbuf, label, color)	apex::gfx::ScopedCommandBufferLabel CONCAT(__scoped_cmdbuf_label_,__LINE__) ((cmdbuf), (label), (color))
