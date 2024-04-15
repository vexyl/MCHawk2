#include "TaskManager.hpp"

TaskManager::~TaskManager()
{
	m_taskList.clear();
}

void TaskManager::UpdateTasks()
{
	TaskList::iterator it = m_taskList.begin();
	while (it != m_taskList.end()) {
		std::shared_ptr<Task> currTask = (*it);

		if (currTask->GetState() == Task::kUnitialized) {
			currTask->OnInit();
		}

		if (currTask->GetState() == Task::kRunning) {
			currTask->OnUpdate();
		}

		if (currTask->IsDead()) {
			switch (currTask->GetState()) {
				case Task::kFinished:
				{
					currTask->OnFinished();
					break;
				}
				case Task::kFailed:
				{
					currTask->OnFailed();
					break;
				}
				case Task::kAborted:
				{
					currTask->OnAbort();
					break;
				}
			}

			it = m_taskList.erase(it);
			continue;
		}
		++it;
	}
}

std::weak_ptr<Task> TaskManager::AttachTask(std::shared_ptr<Task> task)
{
	m_taskList.push_back(task);
	return std::weak_ptr<Task>(task);
}

void TaskManager::AbortAllTasks(bool immediate)
{
	for (auto& task : m_taskList) {
		task->m_state = Task::kAborted;
	}
}
