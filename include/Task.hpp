#ifndef TASK_H_
#define TASK_H_

#include <list>
#include <memory>

class Task {
	friend class TaskManager;

public:
	enum State {
		kUnitialized = 0,
		kRemoved,
		kRunning,
		kPaused,
		kFinished,
		kFailed,
		kAborted
	};

	Task() : m_state(kUnitialized) {}
	virtual ~Task() {}

	inline void Finish() { m_state = kFinished; }
	inline void Fail() { m_state = kFailed; }

	inline void Pause()
	{
		if (m_state == kRunning)
			m_state = kPaused;
	}

	inline void Resume()
	{
		if (m_state == kPaused)
			m_state = kRunning;
	}

	State GetState() const { return m_state; }
	bool IsAlive() const { return (m_state == kRunning || m_state == kPaused); }
	bool IsDead() const {
		return (m_state == kFinished || m_state == kFailed || m_state == kAborted);
	}

	bool IsRemoved() const { return m_state == kRemoved; }
	bool IsPaused() const { return m_state == kPaused; }

private:
	State m_state;

	void SetState(State newState) { m_state = newState; }

protected:
	virtual void OnInit() { m_state = kRunning; }
	virtual void OnUpdate() = 0;
	virtual void OnFinished() {}
	virtual void OnFailed() {}
	virtual void OnAbort() {}
};

#endif // TASK_H_