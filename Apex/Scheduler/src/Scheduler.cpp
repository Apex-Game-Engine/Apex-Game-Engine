#include "Scheduler.h"

namespace apex::sched {

	bool TaskRegistry::RegisterTask(const std::string& task_name, TaskDefinition task_def)
	{
		auto[it, success] = m_tasks.try_emplace(task_name, task_def);
		return success;
	}

	const TaskDefinition* TaskRegistry::TryGetTask(const std::string& task_name) const
	{
		const auto it = m_tasks.find(task_name); 
		if (it == m_tasks.end())
			return nullptr;
		return &it->second;
	}

	void TaskQueue::PushBack(const TaskDefinition* definition)
	{
		std::lock_guard lock(m_token);
		m_queue.emplace_back(apex_new Task { definition, TaskStatus::Scheduled, (u32)definition->predecessors.size(), nullptr, nullptr });
	}

	void TaskQueue::PushFront(const TaskDefinition* definition)
	{
		std::lock_guard lock(m_token);
		m_queue.emplace_front(apex_new Task { definition, TaskStatus::Scheduled, (u32)definition->predecessors.size(), nullptr, nullptr });
	}

	Task* TaskQueue::PopFront()
	{
		std::lock_guard lock(m_token);
		Task* task = m_queue.front();
		m_queue.pop_front();
		return task;
	}

	bool TaskQueue::Empty() const
	{
		std::lock_guard lock(m_token);
		return m_queue.empty();
	}

	DEFINE_POOL_MEMBERS(Task);
}
