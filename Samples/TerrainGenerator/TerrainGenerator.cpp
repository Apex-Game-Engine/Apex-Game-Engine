#include "Core/Console.h"
#include "Core/Logging.h"
#include "Graphics/GraphicsContext.h"
#include "Math/Matrix4x4.h"
#include "Memory/MemoryManager.h"
#include "Platform/PlatformManager.h"

#include <Windows.h>
#include "renderdoc_app.h"
#include "Platform/Timer.h"

using namespace apex;

bool g_running = true;

// Renderdoc API
RENDERDOC_API_1_6_0 *rdoc_api = nullptr;

struct RenderDocFrameCapture
{
	RenderDocFrameCapture() { if (rdoc_api) rdoc_api->StartFrameCapture(nullptr, nullptr); }
	~RenderDocFrameCapture() { if (rdoc_api) rdoc_api->EndFrameCapture(nullptr, nullptr); }
};

#define RENDERDOC_FRAME_CAPTURE() RenderDocFrameCapture rdoc_frame_capture_##__LINE__

void connect_renderdoc()
{
	if (HMODULE mod = GetModuleHandleA("renderdoc.dll"))
	{
		pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
		int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_6_0, (void**)&rdoc_api);
	}
}

plat::PlatformTimer g_timer;

gfx::Context* g_context;

// Pipelines
gfx::GraphicsPipeline* g_phongPipeline;
gfx::ComputePipeline* g_erosionPipeline;

// Images
gfx::Image* g_heightmaps[2]; // ping-pong buffers
gfx::Image* g_depth;

// Buffers
gfx::Buffer* g_camera;

// Command Buffers
AxArray<gfx::CommandBuffer*> g_graphicsCommandBuffers;
AxArray<gfx::CommandBuffer*> g_computeCommandBuffers;

// Timeline Fence
gfx::Fence* g_fence;

// stage 0 : write Heightmap0
// stage 1 : write Heightmap1, transfer ownership to Graphics queue
// stage 2 : blit Heightmap1, present image
u64 g_stageIndex = 0;

enum Stage
{
	Stage_0,
	Stage_1,
	Stage_2,
	Stage_Count,

	Stage_ComputeMax = Stage_2,
	Stage_Graphics = Stage_2,
};

constexpr static Stage GetStage(u64 stage_index)
{
	return static_cast<Stage>(stage_index % Stage_Count);
}

constexpr static u64 GetNextIndexForStage(Stage stage, u64 stage_index)
{
	return stage_index - (stage_index % Stage_Count) + stage + (Stage_Count * ((stage_index % Stage_Count) > stage));
}

typedef void (*PFN_DeleteGraphicsObject)(void*);

struct AutoDeleteWrapper
{
	AutoDeleteWrapper(PFN_DeleteGraphicsObject deleter, void* obj) : deleter(deleter), obj(obj) {}
	~AutoDeleteWrapper() { if (deleter) deleter(obj); }

	AutoDeleteWrapper(const AutoDeleteWrapper&) = delete;
	AutoDeleteWrapper& operator=(const AutoDeleteWrapper&) = delete;

	AutoDeleteWrapper(AutoDeleteWrapper&& other) noexcept
	{
		*this = std::move(other);
	}

	AutoDeleteWrapper& operator=(AutoDeleteWrapper&& other) noexcept
	{
		deleter = other.deleter;
		obj = other.obj;
		other.deleter = nullptr;
		other.obj = nullptr;
		return *this;
	}

	PFN_DeleteGraphicsObject deleter;
	void* obj;
};

AxArray<AutoDeleteWrapper> g_deletionQueue;

template <typename T> void DefaultDeleter(void* obj) { axDebugFmt("deleting resource : {}", obj); delete static_cast<T*>(obj); }
template <typename T> void AddToGlobalDeletionQueue(T* obj) { g_deletionQueue.emplace_back(DefaultDeleter<T>, obj); }

enum SimulationPhase : u32
{
	SP_Initialize,
	SP_Erode,
};

struct SimulateErosion_PushConstants
{
	f32 fTimeElapsedMicros;
	u32 Phase;
	u32 HeightMapInIdx;
	u32 HeightMapOutIdx;
} g_erosionConstants;

struct CameraMatrices
{
	math::Matrix4x4 view;
	math::Matrix4x4 projection;
	math::Matrix4x4 inverseView;
};

void CreatePipelines(gfx::Device& device)
{
	{
		gfx::ShaderModule* vertexShader = device.CreateShaderModule("standard_phong.vert", R"(X:\ApexGameEngine-Vulkan\build-msvc\Apex\Graphics\spv\standard_phong.vert.spv)");
		gfx::ShaderModule* fragmentShader = device.CreateShaderModule("standard_phong.frag", R"(X:\ApexGameEngine-Vulkan\build-msvc\Apex\Graphics\spv\standard_phong.frag.spv)");
		g_phongPipeline = device.CreateGraphicsPipeline("Standard Phong Pipeline", 
		{
			.shaderStages = {
				.vertexShader = vertexShader,
				.fragmentShader = fragmentShader
			},
		});
		delete vertexShader;
		delete fragmentShader;
	}
	{
		// gfx::ShaderModule* vertexShader = device.CreateShaderModule("")
		
	}
	{
		gfx::ShaderModule* computeShader = device.CreateShaderModule("simulate_erosion.comp", R"(X:\ApexGameEngine-Vulkan\build-msvc\Samples\TerrainGenerator\spv\simulate_erosion.comp.spv)");
		g_erosionPipeline = device.CreateComputePipeline("Simulate Erosion Pipeline", { .computeShader = computeShader });
		delete computeShader;
	}

	AddToGlobalDeletionQueue(g_phongPipeline);
	AddToGlobalDeletionQueue(g_erosionPipeline);
}

void CreateImages(gfx::Device& device)
{
	const gfx::ImageCreateDesc imageDesc {
		.imageType = gfx::ImageType::Image2D,
		.format = gfx::ImageFormat::R32_FLOAT,
		.usageFlags = gfx::ImageUsageFlagBits::Storage | gfx::ImageUsageFlagBits::Sampled | gfx::ImageUsageFlagBits::TransferSrc,
		.dimensions = { 512, 512, 1 },
		.requiredFlags = gfx::MemoryPropertyFlagBits::DeviceLocal,
		.memoryFlags = gfx::MemoryAllocateFlagBits::None,
		.ownerQueue = gfx::QueueType::Compute,
	};

	g_heightmaps[0] = device.CreateImage("Heightmap0", imageDesc);
	g_heightmaps[1] = device.CreateImage("Heightmap1", imageDesc);

	gfx::CommandBuffer* commands = device.AllocateCommandBuffer(gfx::QueueType::Compute, 0, 0);
	commands->Begin();
	for (size_t i = 0; i < 2; i++)
		commands->TransitionImage(g_heightmaps[i],
		                          gfx::ImageLayout::Undefined,				gfx::PipelineStageFlagBits::TopOfPipe,
		                          gfx::AccessFlagBits::None,		gfx::QueueType::Compute, gfx::ImageLayout::General,
		                          gfx::PipelineStageFlagBits::ComputeShader,	gfx::AccessFlagBits::ShaderWrite,	gfx::QueueType::Compute);
	commands->End();

	device.GetQueue(gfx::QueueType::Compute)->SubmitImmediate(commands);
	device.GetQueue(gfx::QueueType::Compute)->WaitForIdle();

	AddToGlobalDeletionQueue(g_heightmaps[0]);
	AddToGlobalDeletionQueue(g_heightmaps[1]);
}

void CreateCameraBuffer(gfx::Device& device)
{
	g_camera = device.CreateBuffer("Camera UBO", {
		.size = sizeof(CameraMatrices),
		.usageFlags =
		gfx::BufferUsageFlagBits::Uniform,
		.requiredFlags =
		gfx::MemoryPropertyFlagBits::DeviceLocal |
		gfx::MemoryPropertyFlagBits::HostVisible |
		gfx::MemoryPropertyFlagBits::HostCoherent,
		.memoryFlags =
		gfx::MemoryAllocateFlagBits::HostAccessSequential,
		.ownerQueue = gfx::QueueType::Graphics,
		.createMapped = true,
	});

	AddToGlobalDeletionQueue(g_camera);
}

void setup(gfx::Device& device)
{
	CreatePipelines(device);
	CreateImages(device);
	CreateCameraBuffer(device);

	device.BindStorageImage(g_heightmaps[0]->GetView());
	device.BindStorageImage(g_heightmaps[1]->GetView());

	const u32 framesInFlight = device.GetFramesInFlight();
	g_graphicsCommandBuffers.reserve(framesInFlight);
	g_computeCommandBuffers.reserve(framesInFlight * Stage_ComputeMax);

	for (u32 frame = 0; frame < framesInFlight; frame++)
	{
		g_graphicsCommandBuffers.emplace_back(device.AllocateCommandBuffer(gfx::QueueType::Graphics, frame, 0));

		for (u32 stage = 0; stage < Stage_ComputeMax; stage++)
		{
			g_computeCommandBuffers.emplace_back(device.AllocateCommandBuffer(gfx::QueueType::Compute, frame, 0));
		}
	}

	g_fence = device.CreateFence("Compute Timeline", 0);
	AddToGlobalDeletionQueue(g_fence);
}

void compute(gfx::Device& device)
{
	if (!axVerify(GetStage(g_stageIndex) != Stage_2))
	{
		return;
	}

	const u32 ping = GetStage(g_stageIndex) == Stage_0;
	const u32 pong = GetStage(g_stageIndex) == Stage_1;

	g_timer.End();
	g_erosionConstants.fTimeElapsedMicros = static_cast<f32>(g_timer.GetElapsedMicroseconds());
	g_erosionConstants.Phase = SP_Initialize;
	g_erosionConstants.HeightMapInIdx = g_heightmaps[ping]->GetView()->GetBindlessIndex(gfx::BindlessDescriptorType::StorageImage);
	g_erosionConstants.HeightMapOutIdx = g_heightmaps[pong]->GetView()->GetBindlessIndex(gfx::BindlessDescriptorType::StorageImage);

	const u32 frameCount = device.GetFramesInFlight();
	const u32 frameIndex = device.GetCurrentFrameIndex();

	gfx::CommandBuffer* commands = g_computeCommandBuffers[frameIndex * Stage_ComputeMax + GetStage(g_stageIndex)];

	g_fence->Wait(g_stageIndex);

	commands->Begin();

	commands->BindGlobalDescriptorSets();

	if (g_stageIndex > 0 && GetStage(g_stageIndex) == Stage_0)
	{
		commands->TransitionImage(g_heightmaps[1],
			gfx::ImageLayout::TransferSrcOptimal, gfx::PipelineStageFlagBits::Transfer, gfx::AccessFlagBits::TransferRead, gfx::QueueType::Graphics,
			gfx::ImageLayout::General, gfx::PipelineStageFlagBits::ComputeShader, gfx::AccessFlagBits::ShaderRead, gfx::QueueType::Compute);
	}

	commands->BindComputePipeline(g_erosionPipeline);
	commands->PushConstants(g_erosionConstants);
	commands->Dispatch({ 512 / 16, 512 / 16, 1 });

	if (GetStage(g_stageIndex) == Stage_1)
	{
		commands->TransitionImage(g_heightmaps[1],
			gfx::ImageLayout::General, gfx::PipelineStageFlagBits::ComputeShader, gfx::AccessFlagBits::ShaderWrite, gfx::QueueType::Compute,
			gfx::ImageLayout::TransferSrcOptimal, gfx::PipelineStageFlagBits::Transfer, gfx::AccessFlagBits::TransferRead, gfx::QueueType::Graphics);
	}

	commands->End();

	if (g_stageIndex > 0 && GetStage(g_stageIndex) == Stage_0)
		device.GetQueue(gfx::QueueType::Compute)->SubmitCommandBuffer(commands, g_fence, g_stageIndex, gfx::PipelineStageFlagBits::Transfer, g_stageIndex + 1);
	else
		device.GetQueue(gfx::QueueType::Compute)->SubmitCommandBuffer(commands, g_fence, g_stageIndex, gfx::PipelineStageFlagBits::ComputeShader, g_stageIndex + 1);

	g_stageIndex++;
}

void draw(gfx::Device& device)
{
	if (!axVerify(GetStage(g_stageIndex) == Stage_2))
	{
		return;
	}

	const u32 currentFrameIndex = device.GetCurrentFrameIndex();

	const gfx::Image* swapchainImage = device.AcquireNextImage();

	gfx::CommandBuffer* commands = g_graphicsCommandBuffers[currentFrameIndex];

	commands->Begin();

	commands->TransitionImage(swapchainImage,
		gfx::ImageLayout::Undefined, gfx::PipelineStageFlagBits::Transfer, gfx::AccessFlagBits::None, gfx::QueueType::Graphics,
		gfx::ImageLayout::TransferDstOptimal, gfx::PipelineStageFlagBits::Transfer, gfx::AccessFlagBits::TransferWrite, gfx::QueueType::Graphics);

	commands->TransitionImage(g_heightmaps[1],
		gfx::ImageLayout::General, gfx::PipelineStageFlagBits::ComputeShader, gfx::AccessFlagBits::ShaderWrite, gfx::QueueType::Compute,
		gfx::ImageLayout::TransferSrcOptimal, gfx::PipelineStageFlagBits::Transfer, gfx::AccessFlagBits::TransferRead, gfx::QueueType::Graphics);

	commands->BlitImage(g_heightmaps[1], gfx::ImageLayout::TransferSrcOptimal, swapchainImage, gfx::ImageLayout::TransferDstOptimal);

	commands->TransitionImage(swapchainImage,
		gfx::ImageLayout::TransferDstOptimal, gfx::PipelineStageFlagBits::Transfer, gfx::AccessFlagBits::TransferWrite, gfx::QueueType::Graphics,
		gfx::ImageLayout::PresentSrcOptimal, gfx::PipelineStageFlagBits::BottomOfPipe, gfx::AccessFlagBits::None, gfx::QueueType::Graphics);

	commands->TransitionImage(g_heightmaps[1],
		gfx::ImageLayout::TransferSrcOptimal, gfx::PipelineStageFlagBits::Transfer, gfx::AccessFlagBits::TransferRead, gfx::QueueType::Graphics,
		gfx::ImageLayout::General, gfx::PipelineStageFlagBits::ComputeShader, gfx::AccessFlagBits::ShaderRead, gfx::QueueType::Compute);

	commands->End();

	const gfx::QueueSubmitDesc submitDesc {
		.commandBuffers = apex::make_array_ref<gfx::CommandBuffer*>(&commands, 1),
		.fence = g_fence,
		.fenceWaitStageMask = gfx::PipelineStageFlagBits::ComputeShader,
		.fenceWaitValue = g_stageIndex,
		.fenceSignalValue = g_stageIndex + 1,
		.waitImageAcquired = true,
		.imageAcquiredWaitStageMask = gfx::PipelineStageFlagBits::Transfer,
		.signalRenderComplete = true
	};

	device.GetQueue(gfx::QueueType::Graphics)->SubmitCommandBuffers(submitDesc);

	g_stageIndex++;
	
	device.GetQueue(gfx::QueueType::Graphics)->Present();
}

int main(int argc, char* argv[])
{
	g_timer.Start();

	logging::Logger::Init();
	Console console("Terrain Generator");
	logging::Logger::AttachConsole(&console);

	mem::MemoryManager::initialize({ .numFramesInFlight = 3 });
	plat::PlatformManager::Init({ .windowParams = { .width = 512, .height = 512, .title = "Terrain Generator" } });
	gfx::Context ctx = gfx::Context::CreateContext(gfx::ContextApi::Vulkan);
	plat::PlatformWindow& mainWindow = plat::PlatformManager::GetMainWindow();
	ctx.Init(mainWindow);
	g_context = &ctx;

	mainWindow.Show();

	connect_renderdoc();

	g_deletionQueue.reserve(512);

	gfx::Device& device = *ctx.GetDevice();
	{
		setup(device);
		RENDERDOC_FRAME_CAPTURE();
		compute(device);
		compute(device);
		draw(device);
	}

	while (g_running)
	{
		plat::PlatformManager::PollEvents();
		g_running = !mainWindow.ShouldQuit();

		if (!g_running)
		{
			break;
		}

		compute(device);
		compute(device);
		draw(device);
	}
	
	ctx.GetDevice()->WaitForIdle();

	g_deletionQueue.reset();
	g_graphicsCommandBuffers.reset();
	g_computeCommandBuffers.reset();

	ctx.Shutdown();
	plat::PlatformManager::Shutdown();
	mem::MemoryManager::shutdown();
	return 0;
}
