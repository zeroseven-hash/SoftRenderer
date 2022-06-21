#pragma once
#include<functional>
#include<thread>
#include<condition_variable>
#include<mutex>
#include<vector>
#include<queue>
#include<future>


template<typename T>
struct alignas(64) ThreadDataWrapper
{
	T data_;
};
class ThreadPool
{
public:
	using Task = std::function<void()>;
	
	static void Init(uint32_t thread_num)
	{
		Get().Start(thread_num);
	}
	
	static ThreadPool& Get()
	{
		static ThreadPool s_instance;
		return s_instance;
	}

	~ThreadPool()
	{
		Stop();
	}

	static uint32_t GetThreadNums()
	{
		return Get().m_thread_num;
	}
	
	template<typename T, typename ...Args>
	static auto PushTask(T task, Args&&... args)->std::future<decltype(task(args...))>
	{
		auto& s_instance = ThreadPool::Get();
		auto wrapper = std::make_shared<std::packaged_task<decltype(task(args...))()>>(std::bind(std::forward<T>(task), std::forward<Args>(args)...));


		{
			std::unique_lock<std::mutex> lock(s_instance.m_mutex);
			s_instance.m_tasks.emplace([wrapper] {
				(*wrapper)();
				});
		}
		s_instance.m_signal.notify_one();
		return wrapper->get_future(); 
	}


private:
	ThreadPool() = default;
	void Start(uint32_t thread_nums)
	{
		m_thread_num = thread_nums;
		for (uint32_t i = 0; i < thread_nums; i++)
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
		for (auto& thread : m_threads) thread.join();
	}
private:

	std::vector<std::thread> m_threads;
	std::queue<Task> m_tasks;
	std::condition_variable m_signal;
	std::mutex m_mutex;
	bool m_stop = false;
	uint32_t m_thread_num=1;

};
