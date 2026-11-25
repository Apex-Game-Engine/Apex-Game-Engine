#include "Core/Console.h"
#include "Core/Logging.h"
#include "Graphics/AxImGui.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/Renderer.h"
#include "Math/Random.h"
#include "Memory/MemoryManager.h"
#include "Mount/Mount.h"
#include "Platform/PlatformManager.h"
#include "Platform/Timer.h"

using namespace apex;

void Main_Prologue(int argc, char** argv)
{
	(void)argc; (void)argv;
	logs::Logger::Init(APEX_USE_CONSOLE);
	mem::MemoryManager::initialize({ 0, 3 });
	StringTable::Init();
	math::Random::Init();
#if APEX_USE_GUI
	plat::PlatformManager::Init({ 1366, 768, APEX_APPLICATION_TITLE " v" APEX_APPLICATION_VERSION });
	plat::PlatformWindow& mainWindow = plat::PlatformManager::GetMainWindow();
	gfx::Factory::InitContext(gfx::Api::Vulkan, mainWindow);
	AxImGui::Init(mainWindow, gfx::Factory::GetContext());
#endif
	mnt::MountManager::Init();
	// const char* APEX_ROOT = std::getenv("APEX_ROOT");
	AxString APEX_ROOT = plat::PlatformManager::GetEnvVar("APEX_ROOT");
	axCriticalAssertFmt(APEX_ROOT.size(), "APEX_ROOT environment variable not found! Please ensure Apex is installed correctly.");
	APEX_ROOT.Append("/Apex/");
	mnt::Path apexRootPath { APEX_ROOT };
	//mnt::Path exePath{ plat::PlatformManager::GetExecutablePath() };
	//mnt::MountManager::MountDirectory("root"_hs, exePath.ResolveRelative("../../../").GetDirectoryPath());
	mnt::MountManager::MountDirectory("apex_root"_hs, apexRootPath.GetDirectoryPath());
	mnt::MountManager::MountDirectory("root"_hs, plat::PlatformManager::GetWorkingDirectory());
#if APEX_USE_GUI
	mainWindow.Show();
#endif
}

void Main_Epilogue(int argc, char** argv)
{
	(void)argc; (void)argv;
	mnt::MountManager::Shutdown();
#if APEX_USE_GUI
	AxImGui::Shutdown(gfx::Factory::GetContext());
	gfx::Factory::ShutdownContext();
	plat::PlatformManager::Shutdown();
#endif
	StringTable::Shutdown();
	mem::MemoryManager::shutdown();
}

enum class FsmState
{
	Init,
	Update,
	Render,
	Present,
	Shutdown,
};

extern void Setup(gfx::FrameData const& frameData);
extern void Update(plat::Time dt);
extern void Draw(gfx::FrameData const& frameData);
extern void DebugDraw(gfx::FrameData const& frameData);
extern void Teardown();

int main(int argc, char* argv[])
{
	Main_Prologue(argc, argv);
	
	bool running = true;

	plat::PlatformWindow& mainWindow = plat::PlatformManager::GetMainWindow();
	
	{
		gfx::FrameData frameData{ gfx::Factory::GetContext().GetDevice() };
		Setup(frameData);

		plat::Timer timer;
		timer.Start();
		float totalTime = 0.0;

		gfx::Context& ctx = gfx::Factory::GetContext();

		while (running)
		{
			// Poll window events
			if (mainWindow.IsMinimized())
			{
				plat::PlatformManager::WaitForEvents();
				continue;
			}
			plat::PlatformManager::PollEvents();
			running = !mainWindow.ShouldQuit();

			// Update timers
			timer.End();
			float curTime = timer.GetElapsedSeconds();
			float frameTime = curTime - totalTime;
			totalTime = curTime;

			// Update
			Update({ totalTime, frameTime });

			// Draw ImGui
			AxImGui::BeginFrame();
			DebugDraw(frameData);
			AxImGui::EndFrame();

			// Draw
			frameData.BeginFrame();

			gfx::CommandBuffer* cmdbuf = frameData.GetCommandBuffer();

			cmdbuf->BindGlobalDescriptorSets();

			Draw(frameData);

			cmdbuf->BeginRendering(frameData.GetBackBufferImage()->GetView(), nullptr, gfx::LoadOp::Load);
			AxImGui::Render(cmdbuf);
			cmdbuf->EndRendering();

			frameData.EndFrame();
		}

		gfx::Factory::GetContext().GetDevice()->WaitForIdle();
	}

	Teardown();

	Main_Epilogue(argc, argv);

	return 0;
}