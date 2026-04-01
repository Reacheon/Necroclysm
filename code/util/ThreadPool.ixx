export module ThreadPool;

import std;

export class ThreadPool
{
public:
    ThreadPool(size_t num_threads);
    ~ThreadPool();
    void addTask(std::function<void()> func);
    size_t getAvailableThreads();
    void waitForThreads();
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    std::condition_variable completionCondition;
    bool stop;
    size_t activeTasks;
    void workerThread();
};

ThreadPool::ThreadPool(size_t numThreads) : stop(false), activeTasks(0)
{
    for (size_t i = 0; i < numThreads; ++i)
    {
        workers.emplace_back([this] { workerThread(); });
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread& worker : workers)
    {
        worker.join();
    }
}

void ThreadPool::addTask(std::function<void()> func)
{
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        if (stop) throw std::runtime_error("ThreadPool is stopping");
        tasks.emplace(std::move(func));
        ++activeTasks;
    }
    condition.notify_one();
}

size_t ThreadPool::getAvailableThreads()
{
    std::unique_lock<std::mutex> lock(queueMutex);
    return workers.size() - activeTasks;
}

void ThreadPool::waitForThreads()
{
    std::unique_lock<std::mutex> lock(queueMutex);
    completionCondition.wait(lock, [this] { return activeTasks == 0; });
}

void ThreadPool::workerThread()
{
    while (true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this] { return stop || !tasks.empty(); });
            if (stop && tasks.empty()) 
            {
                return;
            }
            task = std::move(tasks.front());
            tasks.pop();
        }
        task();
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            --activeTasks;
            if (activeTasks == 0) 
            {
                completionCondition.notify_all();
            }
        }
    }
}
