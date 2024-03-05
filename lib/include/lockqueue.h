#pragma once
#include <queue>
#include <thread>
#include <mutex>                    // pthread_mutex_t
#include <condition_variable>       // pthread_condition_t

// 异步日志写日志的日志队列
template<typename T>
class LockQueue{
public:
    void Push(const T& data); // 多个worker工作线程都会写日志queue
    T Pop();                 // 一个consumer线程在读取日志

private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condvariable;
};

template<typename T>
void LockQueue<T>::Push(const T& data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(data);
    m_condvariable.notify_one();
}


template<typename T>
T LockQueue<T>::Pop()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    while (m_queue.empty())
    {
        // 日志队列为空, 线程进入wait
        m_condvariable.wait(lock);
    }

    T data = m_queue.front();
    m_queue.pop();
    return data;
}//锁自动释放
