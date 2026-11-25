#pragma once

#include <spirv_reflect.h>
#include <vulkan/vulkan_core.h>
#include <vma.h>

#include "Containers/AxArray.h"
#include "Graphics/GraphicsContext.h"
#include "Math/Color.h"
#include "Memory/UniquePtr.h"

namespace apex::plat
{
	class PlatformWindow;
}

namespace apex {
namespace gfx {
	// Forward Declarations
	class VulkanContext;
	class VulkanContextImpl;
	class VulkanDevice;
	class VulkanQueue;
	class VulkanCommandBuffer;
	class VulkanBuffer;
	class VulkanImage;
	class VulkanImageView;
	class VulkanComputePipeline;
	class VulkanGraphicsPipeline;
	class VulkanShaderModule;
	class VulkanFence;

	struct VulkanPhysicalDeviceFeatures
	{
		void SetupChain(void* pNext = nullptr)
		{
			features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			features.pNext = &features11;
			features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
			features11.pNext = &features12;
			features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
			features12.pNext = &features13;
			features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
			features13.pNext = pNext;
		}

		VkPhysicalDeviceFeatures2        features   {};
		VkPhysicalDeviceVulkan11Features features11 {};
		VkPhysicalDeviceVulkan12Features features12 {};
		VkPhysicalDeviceVulkan13Features features13 {};
	};

	struct VulkanPhysicalDeviceProperties
	{
		void SetupChain(void* pNext = nullptr)
		{
			properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
			properties.pNext = &properties11;
			properties11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
			properties11.pNext = &properties12;
			properties12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
			properties12.pNext = &properties13;
			properties13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES;
			properties13.pNext = pNext;
		}

		VkPhysicalDeviceProperties2        properties   {};
		VkPhysicalDeviceVulkan11Properties properties11 {};
		VkPhysicalDeviceVulkan12Properties properties12 {};
		VkPhysicalDeviceVulkan13Properties properties13 {};
	};

	struct VulkanQueueInfo
	{
		static constexpr u32 kInvalidQueueFamilyIndex = 0xfffffff;
		u32     familyIndex : 28 = kInvalidQueueFamilyIndex;
		bool    supportsGraphics : 1 = false;
		bool    supportsPresent  : 1 = false;
		bool    supportsCompute  : 1 = false;
		bool    supportsTransfer : 1 = false;
	};

	struct VulkanGlobalDescriptor
	{
		u32 binding : 3;
		u32	index : 29;
	};

	class VulkanCommandBuffer : public CommandBuffer
	{
	public:
		VulkanCommandBuffer(VulkanDevice const* device, VkCommandPool command_pool, VkCommandBuffer command_buffer, QueueType queue)
		: m_commandPool(command_pool) , m_commandBuffer(command_buffer), m_queue(queue)
		{}

		~VulkanCommandBuffer() override;

		NON_COPYABLE(VulkanCommandBuffer);

		void* GetNativeHandle() override { return m_commandBuffer; }
		QueueType GetQueueType() const override { return m_queue; }

		void Begin() override;
		void End() override;

		void BindGlobalDescriptorSets() override;

		void BindComputePipeline(ComputePipeline const* pipeline) override;
		void Dispatch(Dim3D group_counts) override;

		void BeginRendering(ImageView const* color_image_view, ImageView const* depth_stencil_image_view, LoadOp load_op) override;
		void BeginRendering(AxSpan<ImageView const*> color_image_views, ImageView const* depth_stencil_image_view, LoadOp load_op) override;
		void EndRendering() override;
		void BindGraphicsPipeline(GraphicsPipeline const* pipeline) override;
		void BindDescriptorSet(DescriptorSet const& descriptor_set, GraphicsPipeline const* pipeline) override;
		void BindDescriptorSet(DescriptorSet const* descriptor_set, ComputePipeline const* pipeline) override {}
		void PushConstants(AxSpan<const char> const& bytes) override;
		void SetViewport(Viewport viewport) override;
		void SetScissor(Rect2D scissor) override;
		void SetClearColor(Color4f rgba) override;
		void Clear() override { axAssertFmt(false, "Function '%s' is not implemented!", __FUNCTION__); }
		void BindVertexBuffer(VertexBuffer const* buffer) override;
		void BindIndexBuffer(Buffer const* buffer) override;
		void Draw(u32 vertex_count) override;
		void DrawIndexed(u32 index_count) override;
		void DrawIndirect() override { axDebug(__FUNCTION__); }
		void TransitionImageCrossQueueFrom(const Image* image,
							ImageLayout old_layout, PipelineStageFlags src_stage_mask, AccessFlags src_access_mask, QueueType src_queue,
							ImageLayout new_layout, PipelineStageFlags dst_stage_mask, AccessFlags dst_access_mask) override;
		void TransitionImageCrossQueueTo(const Image* image,
							ImageLayout old_layout, PipelineStageFlags src_stage_mask, AccessFlags src_access_mask,
							ImageLayout new_layout, PipelineStageFlags dst_stage_mask, AccessFlags dst_access_mask, QueueType dst_queue) override;
		void TransitionImage(const Image* image,
							ImageLayout old_layout, PipelineStageFlags src_stage_mask, AccessFlags src_access_mask,
							ImageLayout new_layout, PipelineStageFlags dst_stage_mask, AccessFlags dst_access_mask) override;
		void TransitionImage(const Image* image, ImageLayout old_layout, ImageLayout new_layout) override;
		void Barrier() override { axDebug(__FUNCTION__); }
		void InsertMemoryBarrier(PipelineStageFlags src_stage_mask, AccessFlags src_access_mask,
		                         PipelineStageFlags dst_stage_mask, AccessFlags dst_access_mask) override;

		void BlitImage(const Image* src, ImageLayout src_layout, const Image* dst, ImageLayout dst_layout) override;
		void CopyBuffer(const Buffer* src, const Buffer* dst) override;
		void CopyImageToBuffer(const Image* src, const Buffer* dst) override { axDebug(__FUNCTION__); }
		void CopyImage(const Image* src, ImageLayout src_layout, const Image* dst, ImageLayout dst_layout) override;
		void CopyBufferToImage(const Buffer* src, const Image* dst, ImageLayout layout) override;
		void CopyQueryResults() override { axDebug(__FUNCTION__); }

		void PushLabel(const char* label_str, math::Vector4 const& color) override;
		void PopLabel() override;

	private:
		VkCommandPool			m_commandPool;
		VkCommandBuffer			m_commandBuffer;
		QueueType				m_queue;

		Color4f					m_clearColor { 0, 0, 0, 0 };

		friend class VulkanDevice;
	};

	class VulkanBuffer : public Buffer
	{
	public:
		VulkanBuffer(DEBUG_ONLY(const char* name,) VkBuffer buffer, VkBufferCreateInfo const& create_info, VmaAllocation allocation, VmaAllocationInfo const& allocation_info)
			: Buffer(DEBUG_IF_ELSE(name,nullptr)), m_buffer(buffer), m_allocation(allocation), m_allocationInfo(allocation_info)
			, m_usage(create_info.usage), m_globalDescriptor{ GlobalDescriptorBinding::COUNT, 0 }
		{}

		~VulkanBuffer() override;

		NON_COPYABLE(VulkanBuffer);

		VulkanBuffer(VulkanBuffer&& other) noexcept
			: Buffer(DEBUG_IF_ELSE(other.GetName(),nullptr)), m_buffer(other.m_buffer), m_allocation(other.m_allocation), m_allocationInfo(other.m_allocationInfo)
			, m_usage(other.m_usage), m_globalDescriptor{ GlobalDescriptorBinding::COUNT, 0 }
		{
		}

		NON_MOVE_ASSIGNABLE(VulkanBuffer);

		void* GetMappedPointer() const override { return m_allocationInfo.pMappedData; }

		bool IsValid() const override { return m_buffer != nullptr; }

		size_t GetSize() const override { return m_allocationInfo.size; }
		size_t GetOffset() const { return m_allocationInfo.offset; }
		GlobalDescriptorBinding GetGlobalDescriptorBinding() const override { return static_cast<GlobalDescriptorBinding::Value>(m_globalDescriptor.binding); }
		u16 GetGlobalDescriptorIndex() const override { return m_globalDescriptor.index; }

		// Vulkan specific methods
		VkBuffer GetNativeHandle() const { return m_buffer; }
		VmaAllocation GetAllocation() const { return m_allocation; }
		VmaAllocationInfo GetAllocationInfo() const { return m_allocationInfo; }
		VkBufferUsageFlags GetUsageFlags() const { return m_usage; }

	private:
		VkBuffer            m_buffer;
		VmaAllocation       m_allocation;
		VmaAllocationInfo   m_allocationInfo;
		VkBufferUsageFlags	m_usage;
		VulkanGlobalDescriptor m_globalDescriptor;

		friend class VulkanDevice;
	};

	class VulkanImage : public Image
	{
	public:
		VulkanImage(DEBUG_ONLY(const char* name,) VulkanDevice const* device, VkImage image, VkImageCreateInfo const& create_info)
			: Image(DEBUG_IF_ELSE(name,nullptr)), m_device(device), m_image(image), m_allocation(nullptr), m_allocationInfo()
			, m_view(nullptr), m_type(create_info.imageType), m_extent(create_info.extent), m_usage(create_info.usage), m_format(create_info.format)
		{}

		VulkanImage(DEBUG_ONLY(const char* name,) VulkanDevice const* device, VkImage image, VkImageCreateInfo const& create_info, VkImageView view);

		VulkanImage(DEBUG_ONLY(const char* name,)VulkanDevice const* device, VkImage image, VkImageCreateInfo const& create_info, VmaAllocation allocation, VmaAllocationInfo const& allocation_info)
			: Image(DEBUG_IF_ELSE(name,nullptr)), m_device(device), m_image(image), m_allocation(allocation), m_allocationInfo(allocation_info)
			, m_view(nullptr), m_type(create_info.imageType), m_extent(create_info.extent), m_usage(create_info.usage), m_format(create_info.format)
		{}

		VulkanImage(DEBUG_ONLY(const char* name,) VulkanDevice const* device, VkImage image, VkImageCreateInfo const& create_info, VmaAllocation allocation, VmaAllocationInfo const& allocation_info, VkImageView view);

		// Used for wrapping over swapchain images
		VulkanImage(DEBUG_ONLY(const char* name,) VulkanDevice const* device, VkImage image, VkImageView view, VkImageType type, VkExtent3D extent, VkFormat format, VkImageUsageFlags usage);

		~VulkanImage() override;

		NON_COPYABLE(VulkanImage);

		VulkanImage(VulkanImage&& other) noexcept
			: Image(DEBUG_IF_ELSE(other.GetName(),nullptr)), m_device(other.m_device), m_image(other.m_image), m_allocation(other.m_allocation), m_allocationInfo(other.m_allocationInfo)
			, m_view(other.m_view), m_extent(other.m_extent), m_usage(other.m_usage), m_format(other.m_format)
		{
			other.m_device = nullptr;
			other.m_image = VK_NULL_HANDLE;
			other.m_allocation = VK_NULL_HANDLE;
			other.m_view = nullptr;
		}

		NON_MOVE_ASSIGNABLE(VulkanImage);

		bool IsValid() const override { return m_image != nullptr; }
		bool HasView() const override { return m_view != nullptr; }
		const ImageView* GetView() const override;
		ImageView* GetView() override;
		
		size_t GetSize() const { return m_allocationInfo.size; }
		size_t GetOffset() const { return m_allocationInfo.offset; }


		// Vulkan specific methods
		VkImage GetNativeHandle() const { return m_image; }
		VmaAllocation GetAllocation() const { return m_allocation; }
		VmaAllocationInfo GetAllocationInfo() const { return m_allocationInfo; }
		VkExtent3D GetExtent() const { return m_extent; }
		VkFormat GetFormat() const { return m_format; }
		VkImageUsageFlags GetUsageFlags() const { return m_usage; }

	private:
		VkImage				m_image;
		VmaAllocation		m_allocation;
		VmaAllocationInfo	m_allocationInfo;
		VulkanImageView*	m_view;
		// Metadata
		VkImageType         m_type;
		VkExtent3D			m_extent;
		VkImageUsageFlags	m_usage;
		VkFormat            m_format;
		// mip levels
		// array layers

		friend class VulkanDevice;
	};

	class VulkanImageView : public ImageView
	{
	public:
		VulkanImageView(DEBUG_ONLY(const char* name,) VkImageView view, VulkanImage* owner)
			: ImageView(DEBUG_IF_ELSE(name,nullptr)), m_view(view), m_owner(owner), m_globalDescriptor{ GlobalDescriptorBinding::COUNT, 0 }
		{}

		~VulkanImageView() override;

		NON_COPYABLE(VulkanImageView);

		bool IsValid() const override { return m_view != nullptr; }
		const Image* GetOwner() const override;
		Image* GetOwner() override;

		GlobalDescriptorBinding GetGlobalDescriptorBinding() const override { return static_cast<GlobalDescriptorBinding::Value>(m_globalDescriptor.binding); }
		u16 GetGlobalDescriptorIndex() const override { return m_globalDescriptor.index; }

		// Vulkan specific methods
		VulkanDevice const* GetDevice() const { return m_owner->GetDevice(); }
		VkImageView GetNativeHandle() const { return m_view; }

	private:
		VkImageView			   m_view;
		VulkanImage*		   m_owner;
		VulkanGlobalDescriptor m_globalDescriptor;

		friend class VulkanDevice;
		friend class VulkanImage;
	};

	inline const ImageView* VulkanImage::GetView() const { return m_view; }
	inline ImageView* VulkanImage::GetView() { return m_view; }

	inline const Image* VulkanImageView::GetOwner() const { return m_owner; }
	inline Image* VulkanImageView::GetOwner() { return m_owner; }

	class VulkanShaderModule : public ShaderModule
	{
	public:
		VulkanShaderModule(DEBUG_ONLY(const char* name,) VkShaderModule shader, const SpvReflectShaderModule& reflect)
			: ShaderModule(DEBUG_IF_ELSE(name,nullptr)), m_shader(shader), m_reflect(reflect)
		{}

		~VulkanShaderModule() override;

		NON_COPYABLE(VulkanShaderModule);

		VkShaderModule GetNativeHandle() const { return m_shader; }

	private:
		VkShaderModule         m_shader;
		SpvReflectShaderModule m_reflect;

		friend class VulkanDevice;
	};

    class VulkanGraphicsPipeline : public GraphicsPipeline
    {
    public:
		VulkanGraphicsPipeline(DEBUG_ONLY(const char* name,) VkPipeline pipeline, VertexFormat vertex_format)
			: GraphicsPipeline(DEBUG_IF_ELSE(name,nullptr)), m_pipeline(pipeline), m_vertexFormat(vertex_format)
		{
		}

		VulkanGraphicsPipeline(DEBUG_ONLY(const char* name,) VkPipeline pipeline, VkPipelineLayout pipelineLayout, VertexFormat vertex_format)
	        : GraphicsPipeline(DEBUG_IF_ELSE(name,nullptr)), m_pipeline(pipeline), m_pipelineLayout(pipelineLayout), m_vertexFormat(vertex_format)
        {
        }

        VulkanGraphicsPipeline(DEBUG_ONLY(const char* name,) VkPipeline pipeline, VkPipelineLayout pipelineLayout, AxArray<VkDescriptorSetLayout> const& descriptorSetLayouts, VertexFormat vertex_format)
	        : GraphicsPipeline(DEBUG_IF_ELSE(name,nullptr)), m_pipeline(pipeline), m_pipelineLayout(pipelineLayout)
	    	, m_descriptorSetLayouts(descriptorSetLayouts), m_vertexFormat(vertex_format)
        {
        }

		~VulkanGraphicsPipeline() override;

		NON_COPYABLE(VulkanGraphicsPipeline);

		VkPipeline GetNativeHandle() const { return m_pipeline; }
		VkPipelineLayout GetPipelineLayout() const { return m_pipelineLayout; }
		AxArray<VkDescriptorSetLayout> const& GetDescriptorSetLayouts() const { return m_descriptorSetLayouts; }
		VertexFormat const& GetVertexFormat() const { return m_vertexFormat; }

		u64 GetVertexFormatHash() const override {return m_vertexFormat.CalcHash(); }

    private:
        VkPipeline                     m_pipeline {};
        VkPipelineLayout               m_pipelineLayout {};
        AxArray<VkDescriptorSetLayout> m_descriptorSetLayouts;
		VertexFormat                   m_vertexFormat;

        friend class VulkanDevice;
    };

	class VulkanComputePipeline : public ComputePipeline
	{
	public:
		VulkanComputePipeline(DEBUG_ONLY(const char* name,) VkPipeline pipeline)
			: ComputePipeline(DEBUG_IF_ELSE(name,nullptr)), m_pipeline(pipeline)
		{}

		VulkanComputePipeline(DEBUG_ONLY(const char* name,) VkPipeline pipeline, VkPipelineLayout pipelineLayout)
	        : ComputePipeline(DEBUG_IF_ELSE(name,nullptr)), m_pipeline(pipeline), m_pipelineLayout(pipelineLayout)
        {
        }

		VulkanComputePipeline(DEBUG_ONLY(const char* name,) VkPipeline pipeline, VkPipelineLayout pipelineLayout, AxArray<VkDescriptorSetLayout> const& descriptorSetLayouts)
	        : ComputePipeline(DEBUG_IF_ELSE(name,nullptr)), m_pipeline(pipeline), m_pipelineLayout(pipelineLayout),
	          m_descriptorSetLayouts(descriptorSetLayouts)
        {
        }

		~VulkanComputePipeline() override;

		NON_COPYABLE(VulkanComputePipeline);

		VkPipeline GetNativeHandle() const { return m_pipeline; }
		VkPipelineLayout GetPipelineLayout() const { return m_pipelineLayout; }
		AxArray<VkDescriptorSetLayout> const& GetDescriptorSetLayouts() const { return m_descriptorSetLayouts; }

	private:
        VkPipeline                     m_pipeline {};
        VkPipelineLayout               m_pipelineLayout {};
        AxArray<VkDescriptorSetLayout> m_descriptorSetLayouts;

		friend class VulkanDevice;
	};

	class VulkanFence : public Fence
	{
	public:
		VulkanFence(VkSemaphore semaphore, u64 init_value)
			: m_semaphore(semaphore), m_counter(init_value)
		{}

		~VulkanFence() override;

		NON_COPYABLE(VulkanFence);

		void Signal(u64 value) override;
		void Wait(u64 value) override;
		u64 GetValue() const override { return 0; }
		
		VkSemaphore GetSemaphore() const { return m_semaphore; }
		u64 GetAtomicCounterValue() const { return m_counter; }

	private:
		VkSemaphore					m_semaphore {};
		std::atomic_uint64_t		m_counter {};

		friend class VulkanDevice;
	};

	struct VulkanSwapchain
	{
		VkSwapchainKHR       handle {};
		VkSurfaceFormatKHR   surfaceFormat {};
		VkPresentModeKHR     presentMode {};
		VkExtent2D           extent {};
	};

	struct VulkanFrameData
	{
		VkSemaphore			imageAcquiredSema {}; // (binary) signalled when image is acquired from swapchain
		VkSemaphore			renderCompleteSema {}; // (binary) signalled when all rendering commands are completed
		VkSemaphore			timelineSema {}; // (timeline) synchronizes
	};

	class VulkanQueue
	{
	public:
		VulkanQueue()
			: m_queue(VK_NULL_HANDLE), m_type(QueueType::COUNT)
		{}

		VulkanQueue(VkQueue queue, VulkanQueueInfo info, QueueType type, AxSpan<VkCommandPool> command_pools)
			: m_queue(queue), m_info(info), m_type(type), m_submitInfos(128)
		{
			u32 n = min(command_pools.size(), m_commandPools.capacity());
			m_commandPools.SetSize(n);
			for (u32 i = 0; i < n; i++)
			{
				m_commandPools[i] = command_pools[i];
			}
		}

		~VulkanQueue() override;

		NON_COPYABLE(VulkanQueue);

		CommandBuffer* AllocateCommandBuffer(u32 frame_index);
		void ResetCommandBuffers(u32 frame_index);

		void Submit(CommandBuffer* command_buffer);
		void Submit(CommandBuffer* command_buffer, bool wait_image_acquired, PipelineStageFlags wait_stage_mask, bool signal_render_complete);
		void Submit(CommandBuffer* command_buffer, Fence* fence, u64 wait_value, PipelineStageFlags wait_stage_mask, u64 signal_value);
		void Submit(const QueueSubmitDesc& desc);
		void Flush();
		bool Present();
		void WaitForIdle();
		bool CanPresent() const { return m_info.supportsPresent; }
		QueueType GetType() const { return m_type; }

		u32 GetQueueFamilyIndex() const { return m_info.familyIndex; }
		VkQueue GetNativeHandle() const { return m_queue; }

	protected:
		VkQueue							m_queue;
		VulkanQueueInfo					m_info;
		QueueType						m_type;
		AxStaticArray<VkCommandPool, MAX_FRAMES_IN_FLIGHT> m_commandPools;
		AxArray<VkSubmitInfo2>			m_submitInfos;

		friend class VulkanDevice;
	};

	class VulkanDevice
	{
	public:
		VulkanDevice(VulkanContextImpl* context, VkPhysicalDevice physical_device, VkDevice logical_device, VmaAllocator allocator, AxSpan<VulkanQueueInfo> queue_infos);
		~VulkanDevice();

		NON_COPYABLE(VulkanDevice);
		NON_MOVABLE(VulkanDevice);

		VulkanContextImpl*						GetContext() const								{ return m_context; }
		VkPhysicalDevice						GetPhysicalDevice() const						{ return m_physicalDevice; }
		VkDevice								GetLogicalDevice() const						{ return m_logicalDevice; }
		VmaAllocator							GetAllocator() const							{ return m_allocator; }
		VkCommandPool							GetTransferCommandPool() const					{ return m_transferCommandPool; }
		VulkanPhysicalDeviceFeatures const&		GetPhysicalDeviceFeatures() const				{ return m_physicalDeviceFeatures; }
		VulkanPhysicalDeviceProperties const&	GetPhysicalDeviceProperties() const				{ return m_physicalDeviceProperties; }
		VkDescriptorPool						GetDescriptorPool() const						{ return m_descriptorPool; }
		//u32										GetRenderThreadCount() const					{ return m_renderThreadCount; }

		//VkFence									GetRenderFence() const							{ return GetRenderFence(m_currentFrameIndex); }
		//VkSemaphore								GetImageAcquiredSemaphore() const				{ return GetImageAcquiredSemaphore(m_currentFrameIndex); }
		//VkSemaphore								GetRenderCompleteSemaphore() const				{ return GetRenderCompleteSemaphore(m_currentFrameIndex); }
		//VkFence									GetRenderFence(u32 frame) const					{ return m_renderFences[frame]; }
		//VkSemaphore								GetImageAcquiredSemaphore(u32 frame) const		{ return m_imageAcquiredSemaphores[frame]; }
		//VkSemaphore								GetRenderCompleteSemaphore(u32 frame) const		{ return m_renderCompleteSemaphores[frame]; }
		//void									IncrementFramesPresented()						{ m_framesPresentedCount++; m_currentFrameIndex = m_framesPresentedCount % GetMaxFrameCount(); }

		AxSpan<const VkDescriptorSet> GetBindlessDescriptorSets() const { return make_span(m_bindlessDescriptorSets); }
		AxSpan<const VkDescriptorSetLayout> GetBindlessDescriptorSetLayouts() const { return make_span(m_bindlessDescriptorSetLayouts); }
		VkPipelineLayout GetBindlessPipelineLayout() const { return m_bindlessPipelineLayout; }

		VulkanQueue* GetVulkanQueue(QueueType queue_type) { return &m_queues[queue_type]; }
		const VulkanQueue* GetVulkanQueue(QueueType queue_type) const { return &m_queues[queue_type]; }

		const Image* AcquireNextImage();
		void WaitForIdle() const;

		CommandBuffer* AllocateTransferCommandBuffer() const;
		//void ResetCommandBuffers(QueueType queue_idx, u32 frame_idx, u32 thread_idx) const override;
		//void ResetCurrentFrameCommandBuffers() const override;
		//void Submit(QueueType queue, CommandBuffer* command_buffer) const override;
		//void SubmitImmediateCommandBuffer(QueueType queue, CommandBuffer* command_buffer) const override;
		//void Submit(QueueType queue, AxSpan<CommandBuffer> command_buffers) const;

#if !GFX_USE_BINDLESS_DESCRIPTORS
		AxArray<DescriptorSet> AllocateDescriptorSets(GraphicsPipeline* pipeline) const override;
		void UpdateDescriptorSet(DescriptorSet const& descriptor_set) const override;
#endif // !GFX_USE_BINDLESS_DESCRIPTORS

		ShaderModule* CreateShaderModule(const char* name, const char* filepath) const;
		GraphicsPipeline* CreateGraphicsPipeline(const char* name, GraphicsPipelineCreateDesc const& desc) const;
		ComputePipeline* CreateComputePipeline(const char* name, ComputePipelineCreateDesc const& desc) const;

		Buffer* CreateBuffer(const char* name, BufferCreateDesc const& desc);
		Image* CreateImage(const char* name, ImageCreateDesc const& desc);
		//BufferView* CreateBufferView(const char* name, Buffer const* buffer) const;
		ImageView* CreateImageView(const char* name, Image const* image) const;
		ImageView* CreateImageView(const char* name, ImageViewCreateDesc const& desc) const;

		Fence* CreateFence(const char* name, u64 init_value);

		void BindSampledImage(ImageView* image_view);
		void BindStorageImage(ImageView* image_view);
		void BindUniformBuffer(Buffer* buffer, u64 offset, u64 size);
		void BindStorageBuffer(Buffer* buffer, u64 offset, u64 size, u32 index);

		void DestroyShaderModule(ShaderModule* shader) const;
		void DestroyPipeline(GraphicsPipeline* pipeline) const;
		void DestroyPipeline(ComputePipeline* pipeline) const;
		void DestroyBuffer(Buffer* buffer) const;
		void DestroyImage(Image* image) const;
		void DestroyBufferView(BufferView* view) const {}
		void DestroyImageView(ImageView* view) const;
		void DestroyFence(Fence* fence) const;

	protected:
		void DestroySwapchain();
		void CreateBindlessDescriptorResources();
		void CreatePerFrameData();
		void DestroyPerFrameData();
		void CreateTransferCommandPool();
		VkDescriptorSetLayout CreateDescriptorSetLayoutFromShader(VulkanShaderModule const* shader_module, u32 set_idx) const;
		VkPushConstantRange CreatePushConstantRangeFromShader(VulkanShaderModule const* shader_module, u32 push_constant_idx) const;

	private:
		VkPhysicalDevice							m_physicalDevice {};
		VkDevice									m_logicalDevice {};
		VmaAllocator								m_allocator {};
		VkDescriptorPool							m_descriptorPool {};
	#if GFX_USE_BINDLESS_DESCRIPTORS
		AxStaticArray<VkDescriptorSet, 5>			m_bindlessDescriptorSets {};
		AxStaticArray<VkDescriptorSetLayout, 5>		m_bindlessDescriptorSetLayouts {};
		VkPipelineLayout							m_bindlessPipelineLayout {};
		std::atomic_uint32_t						m_nextSampledImageDescriptorIndex {0};
		std::atomic_uint32_t						m_nextStorageImageDescriptorIndex {0};
		std::atomic_uint32_t						m_nextUniformBufferDescriptorIndex {0};
		std::atomic_uint32_t						m_nextStorageBufferDescriptorIndex {0};
	#endif
		VkSampler									m_defaultNearestSampler {};
		VkSampler									m_defaultLinearSampler {};
		VkSampler									m_defaultBilinearSampler {};

		// Queue objects
		AxStaticArray<VulkanQueue, QueueType::COUNT>	m_queues {};

		VkCommandPool								m_transferCommandPool {};
		// Per frame objects
		//template <typename T> using FrameResourceArray = AxStaticArray<T, MAX_FRAMES_IN_FLIGHT>;
		//AxArray<VulkanImage>						m_swapchainImages {};
		//FrameResourceArray<VkFence>					m_renderFences {};
		//FrameResourceArray<VkSemaphore>				m_imageAcquiredSemaphores {};
		//FrameResourceArray<VkSemaphore>				m_renderCompleteSemaphores {};

		// Device meta properties
		VulkanPhysicalDeviceFeatures				m_physicalDeviceFeatures {};
		VulkanPhysicalDeviceProperties				m_physicalDeviceProperties {};
		VkPhysicalDeviceMemoryProperties2			m_physicalDeviceMemoryProperties {};
		AxArray<VkExtensionProperties>				m_availableExtensions {};

		friend class VulkanContextImpl;
	};

	class VulkanContext final : public Context
	{
	public:
		VulkanContext() = default;
		~VulkanContext() override = default;
		
		void Init(const plat::PlatformWindow& window) override;
		void Shutdown() override;
		
		Api GetApi() const override { return Api::Vulkan; }
		void GetDeviceFeatures(DeviceFeatures& device_features) const override;
		void GetDeviceProperties(DeviceProperties& device_properties) const override;

		void ResizeSurface(u32 width, u32 height) const override;

		void ResizeSurface() const;
		VkInstance GetInstance() const;

		ShaderModule* CreateShaderModule(const char* name, const char* filepath) const override { return GetDevice().CreateShaderModule(name, filepath); }
		GraphicsPipeline* CreateGraphicsPipeline(const char* name, GraphicsPipelineCreateDesc const& desc) const override { return GetDevice().CreateGraphicsPipeline(name, desc); }
		ComputePipeline* CreateComputePipeline(const char* name, ComputePipelineCreateDesc const& desc) const override { return GetDevice().CreateComputePipeline(name, desc); }

		Buffer* CreateBuffer(const char* name, BufferCreateDesc const& desc) override { return GetDevice().CreateBuffer(name, desc); }
		Image* CreateImage(const char* name, ImageCreateDesc const& desc) override { return GetDevice().CreateImage(name, desc); }
		ImageView* CreateImageView(const char* name, Image const* image) const { return GetDevice().CreateImageView(name, image); }
		ImageView* CreateImageView(const char* name, ImageViewCreateDesc const& desc) const override { return GetDevice().CreateImageView(name, desc); }

		Fence* CreateFence(const char* name, u64 init_value) override { return GetDevice().CreateFence(name, init_value); }

		void BindSampledImage(ImageView* image_view) override { return GetDevice().BindSampledImage(image_view); }
		void BindStorageImage(ImageView* image_view) override { return GetDevice().BindStorageImage(image_view); }
		void BindUniformBuffer(Buffer* buffer, u64 offset, u64 size) override { return GetDevice().BindUniformBuffer(buffer, offset, size); }
		void BindStorageBuffer(Buffer* buffer, u64 offset, u64 size, u32 index) override { return GetDevice().BindStorageBuffer(buffer, offset, size, index); }

		const Image* AcquireNextImage() override;

		u32							GetTotalFramesPresented() const override		{ return m_framesPresentedCount; }
		u32							GetMaxFrameCount() const override				{ return m_swapchainImageCount; }
		u32							GetCurrentFrameIndex() const override			{ return m_currentFrameIndex; }
		u32							GetCurrentSwapchainImageIndex() const override	{ return m_currentSwapchainImageIndex; }
		Dim2D						GetSurfaceDim() const override					{ return { m_swapchain.extent.width, m_swapchain.extent.height }; }

		VkInstance					GetInstance() const		{ return m_instance; }
		VkSurfaceKHR				GetSurface() const		{ return m_surface; }
		VulkanDevice const&			GetDevice() const		{ return *m_device; }

	protected:
		void CreateDevice(VkPhysicalDevice physical_device, VulkanPhysicalDeviceFeatures const& enabled_device_features);
		void CreateSwapchain(VkSurfaceKHR surface, u32 width, u32 height);

	private:
		VkInstance					m_instance {};
		VkSurfaceKHR				m_surface {};
		UniquePtr<VulkanDevice>		m_device {};
		VkDebugUtilsMessengerEXT	m_debugUtilsMessenger {};

		// Swapchain object
		VulkanSwapchain								m_swapchain {};

		u32							m_renderThreadCount = 1;
		u32							m_swapchainImageCount = 0;
		u32							m_currentFrameIndex = 0;
		u32							m_currentSwapchainImageIndex = 0;
		u32							m_framesPresentedCount = 0;
	};

}
}

