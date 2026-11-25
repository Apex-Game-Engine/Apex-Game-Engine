#include "Scheduler.h"
#include "Core/Logging.h"
#include "Core/Asserts.h"
#include "Memory/MemoryManager.h"
#include "Platform/InputManager.h"
#include "Platform/PlatformManager.h"

using namespace apex;

int main(int argc, char* argv[])
{
	logging::Logger::Init();
	mem::MemoryManager::initialize({});

	sched::TaskRegistry taskRegistry;

	taskRegistry.RegisterTask("AnimationUpdate", {
		.function = [](auto, auto)
		{
			printf("Task: AnimationUpdate Start\n");
			for (int i = 0; i < 100000000; i++) {}
			printf("Task: AnimationUpdate End\n");
			return 0;
		},
	});

	taskRegistry.RegisterTask("ApplyTransforms", {
		.function = [](auto, auto)
		{
			printf("Task: ApplyTransforms Start\n");
			for (int i = 0; i < 100000000; i++) {}
			printf("Task: ApplyTransforms End\n");
			return 0;
		}
	});

	sched::Task::InitPool(64);
	sched::TaskQueue taskQueue;

	for (int i = 0; i < 1000; i++)
	{
		for (int j = 0; j < 25; j++)
			taskQueue.PushBack(taskRegistry.TryGetTask("AnimationUpdate"));
		for (int j = 0; j < 25; j++)
			taskQueue.PushBack(taskRegistry.TryGetTask("ApplyTransforms"));

		while (!taskQueue.Empty())
		{
			sched::Task* pTask = taskQueue.PopFront();
			if (!pTask)
			{
				printf("null task\n");
				continue;
			}

			if (const sched::TaskDefinition* def = pTask->definition)
			{
				def->function(def->payload, def->arena);
			}

			delete pTask;
		}

		printf("Finished iteration %d\n", i);

	}

	sched::Task::ShutdownPool();

	mem::MemoryManager::shutdown();

	return 0;
}