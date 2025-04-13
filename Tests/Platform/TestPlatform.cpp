#include "Core/Logging.h"
#include "Core/Asserts.h"
#include "Memory/MemoryManager.h"
#include "Platform/InputManager.h"
#include "Platform/PlatformManager.h"

using namespace apex;

int main(int argc, char* argv[])
{
	logging::Logger::Init();
	mem::MemoryManager::initialize({ 0, 0 });

	plat::PlatformManager::Init({ 1366, 768, "Apex Platform Test" });
	plat::PlatformManager::GetMainWindow().Show();

	while (!plat::PlatformManager::GetMainWindow().ShouldQuit())
	{
		plat::PlatformManager::PollEvents();
		{
			const plat::GamepadState& gamepad = plat::PlatformManager::GetInputManager().GetGamepad(0);
			fmt::println("Gamepad LS: {:0.4f}, {:0.4f}    RS: {:0.4f}, {:0.4f}", gamepad.leftStick.x, gamepad.leftStick.y, gamepad.rightStick.x, gamepad.rightStick.y);
		}
	}

	plat::PlatformManager::Shutdown();

	mem::MemoryManager::shutdown();
}