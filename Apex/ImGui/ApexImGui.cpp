#include "ApexImGui.h"

#include <vulkan/vk_enum_string_helper.h>

#include "backends/imgui_impl_vulkan.h"
#if APEX_PLATFORM_WIN32
#include "backends/imgui_impl_win32.h"
#endif
#include "Graphics/Vulkan/VulkanContext.h"


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace apex {

	static gfx::ContextApi s_api;

	static void ImGuiInitVulkan(gfx::Context ctx)
	{
		axAssert(ctx.GetApi() == gfx::ContextApi::Vulkan);

		auto vkctx = static_cast<gfx::VulkanContext*>(ctx.GetBase());
		auto vkdevice = static_cast<gfx::VulkanDevice*>(vkctx->GetDevice());

		const VkFormat colorAttachmentFormats[] = { vkdevice->GetSwapchain().surfaceFormat.format };

		const VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			// TODO: Change this to use the user defined attachments
			.colorAttachmentCount = 1,
			.pColorAttachmentFormats = colorAttachmentFormats,
		};

		ImGui_ImplVulkan_InitInfo imguiInitInfo {
			.ApiVersion = VK_API_VERSION_1_3,
			.Instance = vkctx->GetInstance(),
			.PhysicalDevice = vkdevice->GetPhysicalDevice(),
			.Device = vkdevice->GetLogicalDevice(),
			.QueueFamily = vkdevice->GetVulkanQueue(gfx::QueueType::Graphics).GetQueueFamilyIndex(),
			.Queue = vkdevice->GetVulkanQueue(gfx::QueueType::Graphics).GetNativeHandle(),
			.DescriptorPool = nullptr,
			.RenderPass = nullptr,
			.MinImageCount = vkdevice->GetFramesInFlight(),
			.ImageCount = vkdevice->GetFramesInFlight(),
			.MSAASamples = VK_SAMPLE_COUNT_1_BIT,
			.DescriptorPoolSize = 128,
			.UseDynamicRendering = true,
			.PipelineRenderingCreateInfo = pipelineRenderingCreateInfo,
			.CheckVkResultFn = [](VkResult result) { axVerifyFmt(VK_SUCCESS == result, "[ImGui-Vulkan] {}", string_VkResult(result)); }
		};

		ImGui_ImplVulkan_Init(&imguiInitInfo);
	}

	bool AxImGui::Init(plat::PlatformWindow& window, gfx::Context& ctx)
	{
		ImGui::SetAllocatorFunctions(
			[](size_t size, void* UNUSED_ARG(udata)) { return mem::GlobalMemoryOperators::OperatorNew(size); },
			[](void* ptr, void* UNUSED_ARG(udata)) { mem::GlobalMemoryOperators::OperatorDelete(ptr); });
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		#if APEX_PLATFORM_WIN32
		ImGuiPlatformIO& pio = ImGui::GetPlatformIO();
		pio.Platform_CreateVkSurface = [](ImGuiViewport* viewport, ImU64 vk_instance, const void* vk_allocator, ImU64* out_vk_surface)
		{
			const VkWin32SurfaceCreateInfoKHR surfaceCreateInfo {
				.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
				.hinstance = (HINSTANCE)plat::PlatformManager::GetMainWindow().GetOsApplicationHandle(),
				.hwnd = (HWND)viewport->PlatformHandleRaw,
			};

			VkResult result = vkCreateWin32SurfaceKHR((VkInstance)vk_instance, &surfaceCreateInfo, (const VkAllocationCallbacks*)vk_allocator, (VkSurfaceKHR*)out_vk_surface);
			return (int)result;
		};
		#endif

	#if APEX_PLATFORM_WIN32
		ImGui_ImplWin32_Init(window.GetOsHandle());
		plat::PlatformManager::SetUserWindowProc([](auto hWnd, auto msg, auto wParam, auto lParam) { return ImGui_ImplWin32_WndProcHandler((HWND)hWnd, msg, wParam, lParam); });
	#endif

		switch (s_api = ctx.GetApi())
		{
		case gfx::ContextApi::Vulkan:	ImGuiInitVulkan(ctx); break;
		default: return false;
		}

		if (auto as = gfx::AutoSubmitCommandBuffer(ctx, gfx::QueueType::Graphics))
		{
			gfx::CommandBuffer* commands = as.GetCommandBuffer();
			commands->Begin();
			ImGui_ImplVulkan_CreateFontsTexture();
			commands->End();
		}

		return true;
	}

	void AxImGui::Shutdown()
	{
		switch (s_api)
		{
		case gfx::ContextApi::Vulkan:	ImGui_ImplVulkan_Shutdown(); break;
		default: ;
		}

		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void AxImGui::BeginFrame()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void AxImGui::EndFrame()
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGui::Render();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void AxImGui::Render(gfx::CommandBuffer* command_buffer)
	{
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), static_cast<VkCommandBuffer>(command_buffer->GetNativeHandle()));
	}
}
