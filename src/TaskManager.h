#ifndef TASKMANAGER_H_
#define TASKMANAGER_H_

#include "Task.h"

class TaskManager final {
	;
public:
	~TaskManager();

	void UpdateTasks();
	std::weak_ptr<Task> AttachTask(std::shared_ptr<Task> task);
	void AbortAllTasks(bool immediate);

	size_t GetTaskCount() const { return m_taskList.size(); }

private:
	typedef std::list<std::shared_ptr<Task>> TaskList;
	TaskList m_taskList;
};

#endif // TASKMANAGER_H_