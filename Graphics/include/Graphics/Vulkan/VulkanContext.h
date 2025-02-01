#pragma once

#include <spirv_reflect.h>
#include <vma.h>
#include <vulkan/vulkan_core.h>

#include "Containers/AxArray.h"
#include "Graphics/GraphicsContext.h"

namespace apex {
namespace gfx {
	class VulkanImageView;

	class VulkanDevice;
	class VulkanContextImpl;

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

	struct VulkanQueueFamily
	{
		enum : u8
		{
			Graphics,
			Compute,
			Transfer,

			COUNT
		} value;

		operator u8() const { return value; }
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
		VulkanCommandBuffer(VulkanDevice const* device, VkCommandPool command_pool, VkCommandBuffer command_buffer)
		: m_device(device), m_commandPool(command_pool) , m_commandBuffer(command_buffer)
		{}

		~VulkanCommandBuffer() override;

		void* GetNativeHandle() override { return m_commandBuffer; }

		void Begin() override;
		void End() override;

		void BeginRenderPass(const ImageView* color_image_view) override;
		void EndRenderPass() override;
		void BindGraphicsPipeline(GraphicsPipeline const* pipeline) override;
		void BindDescriptorSet(DescriptorSet const& descriptor_set, GraphicsPipeline const* pipeline) override;
		void BindDescriptorSet(DescriptorSet const* descriptor_set, ComputePipeline const* pipeline) override {}
		void SetViewport(Viewport viewport) override;
		void SetScissor(Rect2D scissor) override;
		void Clear() override {  }
		void BindVertexBuffer(Buffer const* buffer) override;
		void BindIndexBuffer(Buffer const* buffer) override;
		void Draw(u32 vertex_count) override;
		void DrawIndexed(u32 index_count) override;
		void DrawIndirect() override { axDebug(__FUNCTION__); }
		void TransitionImage(const Image* image, ImageLayout old_layout, ImageLayout new_layout,
			AccessFlags src_access_flags, AccessFlags dst_access_flags,
			PipelineStageFlags src_stage_flags, PipelineStageFlags dst_stage_flags) override;
		void Barrier() override { axDebug(__FUNCTION__); }

		void BeginComputePass() override { axDebug(__FUNCTION__); }
		void EndComputePass() override { axDebug(__FUNCTION__); }
		void BindComputePipeline(ComputePipeline const* pipeline) override { axDebug(__FUNCTION__); }
		void Dispatch() override { axDebug(__FUNCTION__); }

		void CopyBuffer(const Buffer* dst, const Buffer* src) override;
		void CopyImageToBuffer(const Buffer* dst, const Image* src) override { axDebug(__FUNCTION__); }
		void CopyImage(const Image* dst, const Image* src) override { axDebug(__FUNCTION__); }
		void CopyBufferToImage(const Image* dst, const Buffer* src) override { axDebug(__FUNCTION__); }
		void CopyQueryResults() override { axDebug(__FUNCTION__); }

	private:
		VulkanDevice const* m_device;
		VkCommandPool       m_commandPool;
		VkCommandBuffer     m_commandBuffer;

		friend class VulkanDevice;
	};

	class VulkanBuffer : public Buffer
	{
	public:
		VulkanBuffer(VulkanDevice const* device, VkBuffer buffer, VmaAllocation allocation, VmaAllocationInfo const& allocation_info)
		: m_device(device), m_buffer(buffer), m_allocation(allocation), m_allocationInfo(allocation_info)
		{}

		~VulkanBuffer() override;

		void* GetMappedPointer() const override { return m_allocationInfo.pMappedData; }

		bool IsValid() const override { return m_buffer != nullptr; }

		size_t GetSize() const { return m_allocationInfo.size; }
		size_t GetOffset() const { return m_allocationInfo.offset; }

		// Vulkan specific methods
		VkBuffer GetNativeHandle() const { return m_buffer; }
		VmaAllocation GetAllocation() const { return m_allocation; }
		VmaAllocationInfo GetAllocationInfo() const { return m_allocationInfo; }

	private:
		VulkanDevice const* m_device;
		VkBuffer            m_buffer;
		VmaAllocation       m_allocation;
		VmaAllocationInfo   m_allocationInfo;

		friend class VulkanDevice;
	};

	class VulkanImage : public Image
	{
	public:
		VulkanImage(VulkanDevice const* device, VkImage image, VkImageView view);

		VulkanImage(VulkanDevice const* device, VkImage image)
		: m_device(device), m_image(image), m_allocation(nullptr), m_allocationInfo(), m_view(nullptr)
		{}

		VulkanImage(VulkanDevice const* device, VkImage image, VmaAllocation allocation, VmaAllocationInfo const& allocation_info, VkImageView view);

		VulkanImage(VulkanDevice const* device, VkImage image, VmaAllocation allocation, VmaAllocationInfo const& allocation_info)
		: m_device(device), m_image(image), m_allocation(allocation), m_allocationInfo(allocation_info), m_view(nullptr)
		{}

		~VulkanImage() override;

		bool IsValid() const override { return m_image != nullptr; }
		bool HasView() const override { return m_view != nullptr; }
		const ImageView* GetView() const override;
		ImageView* GetView() override;

		// Vulkan specific methods
		VulkanDevice const* GetDevice() const { return m_device; }
		VkImage GetNativeHandle() const { return m_image; }
		VmaAllocation GetAllocation() const { return m_allocation; }
		VmaAllocationInfo GetAllocationInfo() const { return m_allocationInfo; }

	private:
		VulkanDevice const* m_device;
		VkImage             m_image;
		VmaAllocation       m_allocation;
		VmaAllocationInfo   m_allocationInfo;
		VulkanImageView*    m_view;

		friend class VulkanDevice;
	};

	class VulkanImageView : public ImageView
	{
	public:
		VulkanImageView(VkImageView view, VulkanImage* owner)
		: m_view(view), m_owner(owner)
		{}

		~VulkanImageView() override;

		bool IsValid() const override { return m_view != nullptr; }
		const Image* GetOwner() const override;
		Image* GetOwner() override;

		// Vulkan specific methods
		VulkanDevice const* GetDevice() const { return m_owner->GetDevice(); }
		VkImageView GetNativeHandle() const { return m_view; }

	private:
		VkImageView         m_view;
		VulkanImage*        m_owner;

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
        VulkanGraphicsPipeline(VulkanDevice const* device, VkPipeline pipeline, VkPipelineLayout pipelineLayout, const AxArray<VkDescriptorSetLayout>& descriptorSetLayouts)
	        : m_device(device), m_pipeline(pipeline), m_pipelineLayout(pipelineLayout),
	          m_descriptorSetLayouts(descriptorSetLayouts)
        {
        }

		~VulkanGraphicsPipeline() override;

		VkPipeline GetNativeHandle() const { return m_pipeline; }
		VkPipelineLayout GetLayout() const { return m_pipelineLayout; }

    private:
		VulkanDevice const*            m_device;
        VkPipeline                     m_pipeline;
        VkPipelineLayout               m_pipelineLayout;
        AxArray<VkDescriptorSetLayout> m_descriptorSetLayouts;

        friend class VulkanDevice;
    };

	class VulkanComputePipeline : public ComputePipeline
	{
	public:
		VulkanComputePipeline(VulkanDevice const* device, VkPipeline pipeline, VkPipelineLayout pipelineLayout, const AxArray<VkDescriptorSetLayout>& descriptorSetLayouts)
	        : m_device(device), m_pipeline(pipeline), m_pipelineLayout(pipelineLayout),
	          m_descriptorSetLayouts(descriptorSetLayouts)
        {
        }

		~VulkanComputePipeline() override;

	private:
		VulkanDevice const*            m_device;
        VkPipeline                     m_pipeline;
        VkPipelineLayout               m_pipelineLayout;
        AxArray<VkDescriptorSetLayout> m_descriptorSetLayouts;

		friend class VulkanDevice;
	};

	struct VulkanSwapchain
	{
		VkSwapchainKHR       handle {};
		VkSurfaceFormatKHR   surfaceFormat {};
		VkPresentModeKHR     presentMode {};
		VkExtent2D           extent {};
	};

	struct VulkanDescriptorSetLayout
	{
		VkDescriptorSetLayout layout;
		u32 setIndex;
	};

	class VulkanDevice final : public Device
	{
	public:
		VulkanDevice(VulkanContextImpl& context, VkPhysicalDevice physical_device);
		~VulkanDevice() override;

		VulkanDevice(VulkanDevice const&) = delete;
		VulkanDevice& operator=(VulkanDevice const&) = delete;

		VulkanDevice(VulkanDevice &&) = default;
		VulkanDevice& operator=(VulkanDevice &&) = default;

		VkPhysicalDevice                      GetPhysicalDevice() const                      { return m_physicalDevice; }
		VkDevice                              GetLogicalDevice() const                       { return m_logicalDevice; }
		VmaAllocator                          GetAllocator() const                           { return m_allocator; }
		std::pair<VkQueue, VulkanQueueInfo>   GetQueue(VulkanQueueFamily queue_family) const { return { m_queues[static_cast<size_t>(queue_family)], m_queueInfos[static_cast<size_t>(queue_family)] }; }
		VulkanSwapchain const&                GetSwapchain() const                           { return m_swapchain; }
		VkCommandPool                         GetCommandPool(u32 fidx, u32 tidx) const       { return m_commandPools[fidx * m_renderThreadCount + tidx]; }
		VulkanPhysicalDeviceFeatures const&	  GetPhysicalDeviceFeatures() const              { return m_physicalDeviceFeatures; }
		VulkanPhysicalDeviceProperties const& GetPhysicalDeviceProperties() const            { return m_physicalDeviceProperties; }

		u32 GetFramesInFlight() const override { return m_swapchainImageCount; }
		u32 GetCurrentFrameIndex() const override { return m_currentSwapchainImageIndex; }
		Dim2D GetSurfaceDim() const override { return { m_swapchain.extent.width, m_swapchain.extent.height }; }

		CommandBuffer* AllocateCommandBuffer(u32 queueIdx, u32 frame_index, u32 thread_idx) const override;
		void ResetCommandBuffers(u32 thread_idx) const override;
		void ResetCommandBuffers() const override;
		void SubmitCommandBuffer(DeviceQueue queue, CommandBuffer* command_buffer) const override;
		void SubmitImmediateCommandBuffer(DeviceQueue queue, CommandBuffer* command_buffer) const override;
		//void SubmitCommandBuffers(DeviceQueue queue, AxArrayRef<CommandBuffer> command_buffers) const;

		const Image* AcquireNextImage() override;
		void Present(DeviceQueue queue) override;
		void WaitForQueueIdle(DeviceQueue queue) const override;
		void WaitForIdle() const override;

		AxArray<DescriptorSet> AllocateDescriptorSets(GraphicsPipeline* pipeline) const override;
		void UpdateDescriptorSet(DescriptorSet const& descriptor_set) const override;

		ShaderModule* CreateShaderModule(const char* name, const char* filepath) const override;
		GraphicsPipeline* CreateGraphicsPipeline(const char* name, GraphicsPipelineCreateDesc const& desc) const override;

		Buffer* CreateBuffer(const char* name, BufferCreateDesc const& desc) const override;
		Buffer* CreateVertexBuffer(const char* name, size_t size, const void* initial_data) const override;
		Buffer* CreateIndexBuffer(const char* name, size_t size, const void* initial_data) const override;
		Buffer* CreateStagingBuffer(size_t size) const override;

		//Image* CreateImage(const char* name, ImageCreateDesc const& desc) const override;
		//BufferView* CreateBufferView(const char* name, Buffer const* buffer) const;
		ImageView* CreateImageView(const char* name, Image const* image) const;

		void DestroyShaderModule(ShaderModule* shader) const;
		void DestroyPipeline(GraphicsPipeline* pipeline) const;
		void DestroyPipeline(ComputePipeline* pipeline) const;
		void DestroyBuffer(Buffer* buffer) const;
		void DestroyImage(Image* image) const {}
		void DestroyBufferView(BufferView* view) const {}
		void DestroyImageView(ImageView* view) const {}

	protected:
		void CreateSwapchain(VkSurfaceKHR surface, u32 width, u32 height);
		void DestroySwapchain();
		void CreatePerFrameData();
		void DestroyPerFrameData();
		void CreateCommandPools();
		VkDescriptorSetLayout CreateDescriptorSetLayoutFromShader(VulkanShaderModule const* shader_module, u32 set_idx) const;
		VkPushConstantRange CreatePushConstantRangeFromShader(VulkanShaderModule const* shader_module, u32 push_constant_idx) const;


	private:
		// Device objects
		VkPhysicalDevice                  m_physicalDevice {};
		VkDevice                          m_logicalDevice {};
		VmaAllocator                      m_allocator {};
		VkDescriptorPool                  m_descriptorPool {};

		// Queue objects
		VkQueue                           m_queues[static_cast<size_t>(VulkanQueueFamily::COUNT)];
		VulkanQueueInfo                   m_queueInfos[static_cast<size_t>(VulkanQueueFamily::COUNT)];

		// Swapchain object
		VulkanSwapchain                   m_swapchain {};

		// Per frame objects
		AxArray<VulkanImage>              m_swapchainImages {};
		AxArray<VkCommandPool>            m_commandPools {};
		AxArray<VkFence>                  m_renderFences {};
		AxArray<VkSemaphore>              m_acquireSemaphores {};
		AxArray<VkSemaphore>              m_releaseSemaphores {};

		AxArray<VkSemaphore>              m_recycledSemaphores {};

		// Device meta properties
		VulkanPhysicalDeviceFeatures      m_physicalDeviceFeatures {};
		VulkanPhysicalDeviceProperties    m_physicalDeviceProperties {};
		VkPhysicalDeviceMemoryProperties2 m_physicalDeviceMemoryProperties {};
		AxArray<VkExtensionProperties>    m_availableExtensions {};
		u32                               m_renderThreadCount = 1;
		u32                               m_swapchainImageCount = 0;
		u32                               m_currentSwapchainImageIndex = 0;

		friend class VulkanContextImpl;
	};

	class VulkanContext final : public ContextBase
	{
	public:
		VulkanContext() = default;
		~VulkanContext() override = default;
		
	#if APEX_PLATFORM_WIN32
		void Init(HINSTANCE hinstance, HWND hwnd) override;
	#else
		void Init() override;
	#endif
		void Shutdown() override;
		
		Device* GetDevice() const override;
		void GetDeviceFeatures(DeviceFeatures& device_features) const override;
		void GetDeviceProperties(DeviceProperties& device_properties) const override;

		void ResizeWindow(u32 width, u32 height) const override;

	private:
		VulkanContextImpl* m_pImpl {};
	};

}
}

