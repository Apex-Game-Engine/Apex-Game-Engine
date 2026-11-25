#pragma once

#include "Containers/AxArray.h"
#include "Core/Delegate.h"
#include "Memory/UniquePtr.h"

#include <mutex>
#include <deque>
#include <unordered_map>

#include "Memory/AxPool.h"

namespace apex {
namespace sched {

	struct Arena;

	enum class TaskStatus
	{
		Scheduled,
		Waiting,
		Running,
		Finished,
	};

	struct TaskDefinition
	{
		int(*function)(void*, Arena*);
		void* payload; // must be alive until the job finishes
		Arena* arena;

		std::vector<const TaskDefinition*> predecessors;
		std::vector<const TaskDefinition*> successors;

		const char* name;
	};

	class TaskRegistry
	{
	public:
		bool RegisterTask(const std::string& task_name, TaskDefinition task_def);

		const TaskDefinition* TryGetTask(const std::string& task_name) const;

	private:
		std::unordered_map<std::string, TaskDefinition> m_tasks;
	};

	struct Task
	{
		DECLARE_POOL(Task)

		const TaskDefinition* definition;
		TaskStatus status;
		uint32_t counter;
		Task* waitNext;
		Task* waitPrev;
	};

	class TaskQueue
	{
	public:
		TaskQueue() = default;

		void PushBack(const TaskDefinition* definition);
		void PushFront(const TaskDefinition* definition);
		Task* PopFront();
		bool Empty() const;

	private:
		mutable std::mutex m_token;
		std::condition_variable m_cvar;
		std::deque<Task*> m_queue;
	};

	class WaitQueue
	{
	public:
		void Push(Task* task);
		Task* PopFirstAvailable();

	private:
	};

}
}
