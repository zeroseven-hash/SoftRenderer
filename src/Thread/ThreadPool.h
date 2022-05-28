#pragma once
#include<functional>
#include<thread>
#include<condition_variable>
#include<mutex>
#include<vector>
#include<queue>
#include<future>

#define CORE_NUM 4
template<typename T>
struct alignas(64) ThreadDataWrapper
{
	T data_;
};
class ThreadPool
{
public:
	using Task = std::function<void()>;
	explicit ThreadPool(uint32_t thread_nums)
	{
		Start(thread_nums);
	}
	~ThreadPool()
	{
		Stop();
	}

	void Lock()
	{
		m_mutex.lock();
	}
	void UnLock()
	{
		m_mutex.unlock();
	}
	void Join()
	{
		for (auto& thread : m_threads) thread.join();
	}
	template<typename T, typename ...Args>
	auto PushTask(T task, Args&&... args)->std::future<decltype(task(args...))>
	{
		auto wrapper = std::make_shared<std::packaged_task<decltype(task(args...))()>>(std::bind(std::forward<T>(task), std::forward<Args>(args)...));


		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_tasks.emplace([wrapper] {
				(*wrapper)();
				});
		}
		m_signal.notify_one();
		return wrapper->get_future();
	}
public:
	void Start(uint32_t thread_nums)
	{
		for (int i = 0; i < thread_nums; i++)
		{
			m_threads.emplace_back([&] {

				while (true)
				{
					Task task;
					{
						std::unique_lock<std::mutex> lock(m_mutex);
						m_signal.wait(lock, [&] {return m_stop || !m_tasks.empty(); });
						if (m_stop && m_tasks.empty()) break;

						task = std::move(m_tasks.front());
						m_tasks.pop();
					}
					task();
				}
			});
		}
	}

	void Stop() noexcept
	{
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_stop = true;
		}
		m_signal.notify_all();
		Join();
	}
private:
	std::vector<std::thread> m_threads;
	std::queue<Task> m_tasks;
	std::condition_variable m_signal;
	std::mutex m_mutex;
	bool m_stop = false;
	int m_thread_num;

};
