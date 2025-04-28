#pragma once

#include "gfx_config.h"

#include <spirv_reflect.h>
#include <vulkan/vulkan_core.h>
#include <vma.h>

#include "Containers/AxArray.h"
#include "Graphics/GraphicsContext.h"

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

	class VulkanCommandBuffer : public CommandBuffer
	{
	public:
		VulkanCommandBuffer(VulkanDevice const* device, VkCommandPool command_pool, VkCommandBuffer command_buffer, QueueType queue, u32 thread_idx)
		: m_device(device), m_commandPool(command_pool) , m_commandBuffer(command_buffer), m_queue(queue), m_threadIdx(thread_idx)
		{}

		~VulkanCommandBuffer() override;

		NON_COPYABLE(VulkanCommandBuffer);

		void* GetNativeHandle() override { return m_commandBuffer; }

		void Begin() override;
		void End() override;

		void BindGlobalDescriptorSets() override;

		void BindComputePipeline(ComputePipeline const* pipeline) override;
		void Dispatch(Dim3D group_counts) override;

		void BeginRendering(const ImageView* color_image_view, ImageView const* depth_stencil_image_view) override;
		void EndRendering() override;
		void BindGraphicsPipeline(GraphicsPipeline const* pipeline) override;
		void BindDescriptorSet(DescriptorSet const& descriptor_set, GraphicsPipeline const* pipeline) override;
		void BindDescriptorSet(DescriptorSet const* descriptor_set, ComputePipeline const* pipeline) override {}
		void PushConstants(AxArrayRef<const char> const& bytes) override;
		void SetViewport(Viewport viewport) override;
		void SetScissor(Rect2D scissor) override;
		void Clear() override {  }
		void BindVertexBuffer(Buffer const* buffer) override;
		void BindIndexBuffer(Buffer const* buffer) override;
		void Draw(u32 vertex_count) override;
		void DrawIndexed(u32 index_count) override;
		void DrawIndirect() override { axDebug(__FUNCTION__); }
		void TransitionImage(const Image* image,
							 ImageLayout old_layout, PipelineStageFlags src_stage_mask, AccessFlags src_access_mask, QueueType src_queue,
							 ImageLayout new_layout, PipelineStageFlags dst_stage_mask, AccessFlags dst_access_mask, QueueType dst_queue) override;
		void Barrier() override { axDebug(__FUNCTION__); }
		void InsertMemoryBarrier(PipelineStageFlags src_stage_mask, AccessFlags src_access_mask,
		                         PipelineStageFlags dst_stage_mask, AccessFlags dst_access_mask) override;

		void BlitImage(const Image* src, ImageLayout src_layout, const Image* dst, ImageLayout dst_layout) override;
		void CopyBuffer(const Buffer* src, const Buffer* dst) override;
		void CopyImageToBuffer(const Image* src, const Buffer* dst) override { axDebug(__FUNCTION__); }
		void CopyImage(const Image* src, const Image* dst) override { axDebug(__FUNCTION__); }
		void CopyBufferToImage(const Buffer* src, const Image* dst, ImageLayout layout) override;
		void CopyQueryResults() override { axDebug(__FUNCTION__); }

		void PushLabel(const char* label_str, math::Vector4 const& color) override;
		void PopLabel() override;

	private:
		VulkanDevice const*		m_device;
		VkCommandPool			m_commandPool;
		VkCommandBuffer			m_commandBuffer;
		QueueType				m_queue;
		u32						m_threadIdx;

		friend class VulkanDevice;
	};

	class VulkanBuffer : public Buffer
	{
	public:
		VulkanBuffer(VulkanDevice const* device, VkBuffer buffer, VkBufferCreateInfo const& create_info, VmaAllocation allocation, VmaAllocationInfo const& allocation_info)
		: m_device(device), m_buffer(buffer), m_allocation(allocation), m_allocationInfo(allocation_info), m_usage(create_info.usage)
		{}

		~VulkanBuffer() override;

		NON_COPYABLE(VulkanBuffer);

		void* GetMappedPointer() const override { return m_allocationInfo.pMappedData; }

		bool IsValid() const override { return m_buffer != nullptr; }

		size_t GetSize() const override { return m_allocationInfo.size; }
		size_t GetOffset() const { return m_allocationInfo.offset; }
		u32 GetBindlessIndex(BindlessDescriptorType descriptor_type) const override
		{
			axAssert(descriptor_type == BindlessDescriptorType::UniformBuffer || descriptor_type == BindlessDescriptorType::StorageBuffer);
			return m_bindlessIndices[descriptor_type - BindlessDescriptorType::UniformBuffer];
		}

		// Vulkan specific methods
		VkBuffer GetNativeHandle() const { return m_buffer; }
		VmaAllocation GetAllocation() const { return m_allocation; }
		VmaAllocationInfo GetAllocationInfo() const { return m_allocationInfo; }
		VkBufferUsageFlags GetUsageFlags() const { return m_usage; }

	private:
		VulkanDevice const* m_device;
		VkBuffer            m_buffer;
		VmaAllocation       m_allocation;
		VmaAllocationInfo   m_allocationInfo;
		VkBufferUsageFlags	m_usage;
		u32					m_bindlessIndices[2] { (u32)-1, (u32)-1 }; // 0 - UniformBuffer, 1 - StorageBuffer

		friend class VulkanDevice;
	};

	class VulkanImage : public Image
	{
	public:
		VulkanImage(VulkanDevice const* device, VkImage image, VkImageCreateInfo const& create_info)
		: m_device(device), m_image(image), m_allocation(nullptr), m_allocationInfo(), m_view(nullptr)
		, m_extent(create_info.extent), m_usage(create_info.usage), m_format(create_info.format)
		{}

		VulkanImage(VulkanDevice const* device, VkImage image, VkImageCreateInfo const& create_info, VkImageView view);

		VulkanImage(VulkanDevice const* device, VkImage image, VkImageCreateInfo const& create_info, VmaAllocation allocation, VmaAllocationInfo const& allocation_info)
		: m_device(device), m_image(image), m_allocation(allocation), m_allocationInfo(allocation_info), m_view(nullptr)
		, m_extent(create_info.extent), m_usage(create_info.usage), m_format(create_info.format)
		{}

		VulkanImage(VulkanDevice const* device, VkImage image, VkImageCreateInfo const& create_info, VmaAllocation allocation, VmaAllocationInfo const& allocation_info, VkImageView view);

		// Used for wrapping over swapchain images
		VulkanImage(VulkanDevice const* device, VkImage image, VkImageView view, VkExtent3D extent, VkFormat format, VkImageUsageFlags usage);

		~VulkanImage() override;

		NON_COPYABLE(VulkanImage);

		bool IsValid() const override { return m_image != nullptr; }
		bool HasView() const override { return m_view != nullptr; }
		const ImageView* GetView() const override;
		ImageView* GetView() override;
		
		size_t GetSize() const { return m_allocationInfo.size; }
		size_t GetOffset() const { return m_allocationInfo.offset; }


		// Vulkan specific methods
		VulkanDevice const* GetDevice() const { return m_device; }
		VkImage GetNativeHandle() const { return m_image; }
		VmaAllocation GetAllocation() const { return m_allocation; }
		VmaAllocationInfo GetAllocationInfo() const { return m_allocationInfo; }
		VkExtent3D GetExtent() const { return m_extent; }
		VkFormat GetFormat() const { return m_format; }
		VkImageUsageFlags GetUsageFlags() const { return m_usage; }

	private:
		VulkanDevice const*	m_device;
		VkImage				m_image;
		VmaAllocation		m_allocation;
		VmaAllocationInfo	m_allocationInfo;
		VulkanImageView*	m_view;
		// Metadata
		VkExtent3D			m_extent;
		VkImageUsageFlags	m_usage;
		// image type
		VkFormat            m_format;
		// mip levels
		// array layers

		friend class VulkanDevice;
	};

	class VulkanImageView : public ImageView
	{
	public:
		VulkanImageView(VkImageView view, VulkanImage* owner)
		: m_view(view), m_owner(owner)
		{}

		~VulkanImageView() override;

		NON_COPYABLE(VulkanImageView);

		bool IsValid() const override { return m_view != nullptr; }
		const Image* GetOwner() const override;
		Image* GetOwner() override;
		u32 GetBindlessIndex(BindlessDescriptorType descriptor_type) const override
		{
			axAssert(descriptor_type == BindlessDescriptorType::SampledImage || descriptor_type == BindlessDescriptorType::StorageImage);
			return m_bindlessIndices[descriptor_type];
		}

		// Vulkan specific methods
		VulkanDevice const* GetDevice() const { return m_owner->GetDevice(); }
		VkImageView GetNativeHandle() const { return m_view; }

	private:
		VkImageView         m_view;
		VulkanImage*        m_owner;
		u32					m_bindlessIndices[2] { (u32)-1, (u32)-1 }; // 0 - SampledImage, 1 - StorageImage

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
		VulkanShaderModule(VulkanDevice const* device, VkShaderModule shader, const SpvReflectShaderModule& reflect)
		: m_device(device), m_shader(shader), m_reflect(reflect)
		{}

		~VulkanShaderModule() override;

		NON_COPYABLE(VulkanShaderModule);

		VkShaderModule GetNativeHandle() const { return m_shader; }

	private:
		VulkanDevice const*    m_device;
		VkShaderModule         m_shader;
		SpvReflectShaderModule m_reflect;

		friend class VulkanDevice;
	};

    class VulkanGraphicsPipeline : public GraphicsPipeline
    {
    public:
		VulkanGraphicsPipeline(VulkanDevice const* device, VkPipeline pipeline)
			: m_device(device), m_pipeline(pipeline)
		{
		}

		VulkanGraphicsPipeline(VulkanDevice const* device, VkPipeline pipeline, VkPipelineLayout pipelineLayout)
	        : m_device(device), m_pipeline(pipeline), m_pipelineLayout(pipelineLayout)
        {
        }

        VulkanGraphicsPipeline(VulkanDevice const* device, VkPipeline pipeline, VkPipelineLayout pipelineLayout, AxArray<VkDescriptorSetLayout> const& descriptorSetLayouts)
	        : m_device(device), m_pipeline(pipeline), m_pipelineLayout(pipelineLayout),
	          m_descriptorSetLayouts(descriptorSetLayouts)
        {
        }

		~VulkanGraphicsPipeline() override;

		NON_COPYABLE(VulkanGraphicsPipeline);

		VkPipeline GetNativeHandle() const { return m_pipeline; }
		VkPipelineLayout GetPipelineLayout() const { return m_pipelineLayout; }
		AxArray<VkDescriptorSetLayout> const& GetDescriptorSetLayouts() const { return m_descriptorSetLayouts; }

    private:
		VulkanDevice const*            m_device {};
        VkPipeline                     m_pipeline {};
        VkPipelineLayout               m_pipelineLayout {};
        AxArray<VkDescriptorSetLayout> m_descriptorSetLayouts;

        friend class VulkanDevice;
    };

	class VulkanComputePipeline : public ComputePipeline
	{
	public:
		VulkanComputePipeline(VulkanDevice const* device, VkPipeline pipeline)
			: m_device(device), m_pipeline(pipeline)
		{}

		VulkanComputePipeline(VulkanDevice const* device, VkPipeline pipeline, VkPipelineLayout pipelineLayout)
	        : m_device(device), m_pipeline(pipeline), m_pipelineLayout(pipelineLayout)
        {
        }

		VulkanComputePipeline(VulkanDevice const* device, VkPipeline pipeline, VkPipelineLayout pipelineLayout, AxArray<VkDescriptorSetLayout> const& descriptorSetLayouts)
	        : m_device(device), m_pipeline(pipeline), m_pipelineLayout(pipelineLayout),
	          m_descriptorSetLayouts(descriptorSetLayouts)
        {
        }

		~VulkanComputePipeline() override;

		NON_COPYABLE(VulkanComputePipeline);

		VkPipeline GetNativeHandle() const { return m_pipeline; }
		VkPipelineLayout GetPipelineLayout() const { return m_pipelineLayout; }
		AxArray<VkDescriptorSetLayout> const& GetDescriptorSetLayouts() const { return m_descriptorSetLayouts; }

	private:
		VulkanDevice const*            m_device {};
        VkPipeline                     m_pipeline {};
        VkPipelineLayout               m_pipelineLayout {};
        AxArray<VkDescriptorSetLayout> m_descriptorSetLayouts;

		friend class VulkanDevice;
	};

	class VulkanFence : public Fence
	{
	public:
		VulkanFence(VulkanDevice const* device, VkSemaphore semaphore, u64 init_value)
			: m_device(device), m_semaphore(semaphore), m_counter(init_value)
		{}

		~VulkanFence() override;

		NON_COPYABLE(VulkanFence);

		void Signal(u64 value) override;
		void Wait(u64 value) override;
		u64 GetValue() const override { return 0; }
		
		VkSemaphore GetSemaphore() const { return m_semaphore; }
		u64 GetAtomicCounterValue() const { return m_counter; }

	private:
		VulkanDevice const*			m_device {};
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

	class VulkanQueue : public Queue
	{
	public:
		VulkanQueue()
			: m_device(nullptr), m_queue(VK_NULL_HANDLE), m_type(QueueType::COUNT)
		{}

		VulkanQueue(VulkanDevice* device, VkQueue queue, VulkanQueueInfo info, QueueType type)
			: m_device(device), m_queue(queue), m_info(info), m_type(type), m_submitInfos(128)
		{}

		~VulkanQueue() override = default;

		NON_COPYABLE(VulkanQueue);

		void ResetCommandBuffers(u32 frame_idx, u32 thread_idx) const override;
		void ResetCurrentFrameCommandBuffers() const override { axDebug(__FUNCTION__); }
		void SubmitImmediate(CommandBuffer* command_buffer) override;
		void SubmitCommandBuffer(CommandBuffer* command_buffer) override;
		void SubmitCommandBuffer(CommandBuffer* command_buffer, bool wait_image_acquired, PipelineStageFlags wait_stage_mask, bool signal_render_complete) override;
		void SubmitCommandBuffer(CommandBuffer* command_buffer, Fence* fence, u64 wait_value, PipelineStageFlags wait_stage_mask, u64 signal_value) override;
		void SubmitCommandBuffers(const QueueSubmitDesc& desc) override;
		void Flush() override;
		void Present() override;
		void WaitForIdle() override;
		bool CanPresent() const override { return m_info.supportsPresent; }
		QueueType GetType() const override { return m_type; }

		u32 GetQueueFamilyIndex() const { return m_info.familyIndex; }

	protected:
		VulkanDevice*					m_device;
		VkQueue							m_queue;
		VulkanQueueInfo					m_info;
		QueueType						m_type;
		AxArray<VkSubmitInfo2>			m_submitInfos;

		friend class VulkanDevice;
	};

	class VulkanDevice final : public Device
	{
	public:
		VulkanDevice(VulkanContextImpl* context, VkPhysicalDevice physical_device, VkDevice logical_device, VmaAllocator allocator, AxArrayRef<VulkanQueueInfo> queue_infos);
		~VulkanDevice() override;

		NON_COPYABLE(VulkanDevice);
		NON_MOVABLE(VulkanDevice);

		VulkanContextImpl*						GetContext() const								{ return m_context; }
		VkPhysicalDevice						GetPhysicalDevice() const						{ return m_physicalDevice; }
		VkDevice								GetLogicalDevice() const						{ return m_logicalDevice; }
		VmaAllocator							GetAllocator() const							{ return m_allocator; }
		VulkanSwapchain const&					GetSwapchain() const							{ return m_swapchain; }
		VkCommandPool							GetCommandPool(u32 idx) const					{ return m_commandPools[idx]; }
		VulkanPhysicalDeviceFeatures const&		GetPhysicalDeviceFeatures() const				{ return m_physicalDeviceFeatures; }
		VulkanPhysicalDeviceProperties const&	GetPhysicalDeviceProperties() const				{ return m_physicalDeviceProperties; }
		u32										GetRenderThreadCount() const					{ return m_renderThreadCount; }

		VkFence									GetRenderFence() const							{ return GetRenderFence(m_currentFrameIndex); }
		VkSemaphore								GetImageAcquiredSemaphore() const				{ return GetImageAcquiredSemaphore(m_currentFrameIndex); }
		VkSemaphore								GetRenderCompleteSemaphore() const				{ return GetRenderCompleteSemaphore(m_currentFrameIndex); }
		VkFence									GetRenderFence(u32 frame) const					{ return m_renderFences[frame]; }
		VkSemaphore								GetImageAcquiredSemaphore(u32 frame) const		{ return m_imageAcquiredSemaphores[frame]; }
		VkSemaphore								GetRenderCompleteSemaphore(u32 frame) const		{ return m_renderCompleteSemaphores[frame]; }
		void									IncrementFramesPresented()						{ m_framesPresentedCount++; m_currentFrameIndex = m_framesPresentedCount % GetFramesInFlight(); }

		u32										GetTotalFramesPresented() const override		{ return m_framesPresentedCount; }
		u32										GetFramesInFlight() const override				{ return m_swapchainImageCount; }
		u32										GetCurrentFrameIndex() const override			{ return m_currentFrameIndex; }
		u32										GetCurrentSwapchainImageIndex() const override	{ return m_currentSwapchainImageIndex; }
		Dim2D									GetSurfaceDim() const override					{ return { m_swapchain.extent.width, m_swapchain.extent.height }; }

		AxArrayRef<const VkDescriptorSet> GetBindlessDescriptorSets() const { return make_array_ref(m_bindlessDescriptorSets); }
		AxArrayRef<const VkDescriptorSetLayout> GetBindlessDescriptorSetLayouts() const { return make_array_ref(m_bindlessDescriptorSetLayouts); }
		VkPipelineLayout GetBindlessPipelineLayout() const { return m_bindlessPipelineLayout; }

		u32	CalculateCommandPoolIndex(u32 queue_idx, u32 frame_idx, u32 thread_idx) const
		{
			return queue_idx * MAX_FRAMES_IN_FLIGHT * m_renderThreadCount + frame_idx * m_renderThreadCount + thread_idx;
		}


		Queue* GetQueue(QueueType queue_type) override { return &m_queues[queue_type]; }
		const Queue* GetQueue(QueueType queue_type) const override { return &m_queues[queue_type]; }

		const Image* AcquireNextImage() override;
		void WaitForIdle() const override;

		CommandBuffer* AllocateCommandBuffer(QueueType queue_idx, u32 frame_index, u32 thread_idx) const override;
		//void ResetCommandBuffers(QueueType queue_idx, u32 frame_idx, u32 thread_idx) const override;
		//void ResetCurrentFrameCommandBuffers() const override;
		//void SubmitCommandBuffer(QueueType queue, CommandBuffer* command_buffer) const override;
		//void SubmitImmediateCommandBuffer(QueueType queue, CommandBuffer* command_buffer) const override;
		//void SubmitCommandBuffers(QueueType queue, AxArrayRef<CommandBuffer> command_buffers) const;

		AxArray<DescriptorSet> AllocateDescriptorSets(GraphicsPipeline* pipeline) const override;
		void UpdateDescriptorSet(DescriptorSet const& descriptor_set) const override;

		ShaderModule* CreateShaderModule(const char* name, const char* filepath) const override;
		GraphicsPipeline* CreateGraphicsPipeline(const char* name, GraphicsPipelineCreateDesc const& desc) const override;
		ComputePipeline* CreateComputePipeline(const char* name, ComputePipelineCreateDesc const& desc) const override;

		Buffer* CreateBuffer(const char* name, BufferCreateDesc const& desc) override;
		Buffer* CreateVertexBuffer(const char* name, size_t size, const void* initial_data) override;
		Buffer* CreateIndexBuffer(const char* name, size_t size, const void* initial_data) override;
		Buffer* CreateStagingBuffer(const char* name, size_t size) override;

		Image* CreateImage(const char* name, ImageCreateDesc const& desc) override;
		//BufferView* CreateBufferView(const char* name, Buffer const* buffer) const;
		ImageView* CreateImageView(const char* name, Image const* image) const;
		ImageView* CreateImageView(const char* name, ImageViewCreateDesc const& desc) const override;

		Fence* CreateFence(const char* name, u64 init_value) override;

		void BindSampledImage(ImageView* image_view) override;
		void BindStorageImage(ImageView* image_view) override;
		void BindUniformBuffer(Buffer* buffer) override;
		void BindStorageBuffer(Buffer* buffer) override;

		void DestroyShaderModule(ShaderModule* shader) const;
		void DestroyPipeline(GraphicsPipeline* pipeline) const;
		void DestroyPipeline(ComputePipeline* pipeline) const;
		void DestroyBuffer(Buffer* buffer) const;
		void DestroyImage(Image* image) const;
		void DestroyBufferView(BufferView* view) const {}
		void DestroyImageView(ImageView* view) const;
		void DestroyFence(Fence* fence) const;

	protected:
		void CreateSwapchain(VkSurfaceKHR surface, u32 width, u32 height);
		void DestroySwapchain();
		void CreateBindlessDescriptorResources();
		void CreatePerFrameData();
		void DestroyPerFrameData();
		void CreateCommandPools();
		VkDescriptorSetLayout CreateDescriptorSetLayoutFromShader(VulkanShaderModule const* shader_module, u32 set_idx) const;
		VkPushConstantRange CreatePushConstantRangeFromShader(VulkanShaderModule const* shader_module, u32 push_constant_idx) const;

	private:
		VulkanContextImpl*							m_context;

		// Device objects
		VkPhysicalDevice							m_physicalDevice {};
		VkDevice									m_logicalDevice {};
		VmaAllocator								m_allocator {};
		VkDescriptorPool							m_descriptorPool {};
	#if GFX_USE_BINDLESS_DESCRIPTORS
		AxStaticArray<VkDescriptorSet, 5>			m_bindlessDescriptorSets {};
		AxStaticArray<VkDescriptorSetLayout, 5>		m_bindlessDescriptorSetLayouts {};
		VkPipelineLayout							m_bindlessPipelineLayout {};
		std::atomic_uint32_t						m_nextSampledImageBindingIndex {0};
		std::atomic_uint32_t						m_nextStorageImageBindingIndex {0};
		std::atomic_uint32_t						m_nextUniformBufferBindingIndex {0};
		std::atomic_uint32_t						m_nextStorageBufferBindingIndex {0};
	#endif
		VkSampler									m_defaultNearestSampler {};
		VkSampler									m_defaultLinearSampler {};
		VkSampler									m_defaultBilinearSampler {};

		// Queue objects
		AxStaticArray<VulkanQueue, QueueType::COUNT>	m_queues {};

		using CommandPools = AxStaticArray<VkCommandPool, QueueType::COUNT * MAX_COMMAND_POOLS_PER_QUEUE>;
		CommandPools								m_commandPools {}; // #queues x #frames x #threads

		// Swapchain object
		VulkanSwapchain								m_swapchain {};

		// Per frame objects
		template <typename T> using FrameResourceArray = AxStaticArray<T, MAX_FRAMES_IN_FLIGHT>;
		AxArray<VulkanImage>						m_swapchainImages {};
		FrameResourceArray<VkFence>					m_renderFences {};
		FrameResourceArray<VkSemaphore>				m_imageAcquiredSemaphores {};
		FrameResourceArray<VkSemaphore>				m_renderCompleteSemaphores {};

		// Device meta properties
		VulkanPhysicalDeviceFeatures				m_physicalDeviceFeatures {};
		VulkanPhysicalDeviceProperties				m_physicalDeviceProperties {};
		VkPhysicalDeviceMemoryProperties2			m_physicalDeviceMemoryProperties {};
		AxArray<VkExtensionProperties>				m_availableExtensions {};
		u32											m_renderThreadCount = 1;
		u32											m_swapchainImageCount = 0;
		u32											m_currentFrameIndex = 0;
		u32											m_currentSwapchainImageIndex = 0;
		u32											m_framesPresentedCount = 0;

		friend class VulkanContextImpl;
	};

	class VulkanContext final : public ContextBase
	{
	public:
		VulkanContext() = default;
		~VulkanContext() override = default;
		
		void Init(const plat::PlatformWindow& window) override;
		void Shutdown() override;
		
		Device* GetDevice() const override;
		void GetDeviceFeatures(DeviceFeatures& device_features) const override;
		void GetDeviceProperties(DeviceProperties& device_properties) const override;

		void ResizeSurface(u32 width, u32 height) const override;

		void ResizeSurface() const;

		VulkanContextImpl* GetImpl() const { return m_pImpl; }

	private:
		VulkanContextImpl* m_pImpl {};
	};

}
}

