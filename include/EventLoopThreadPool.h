#pragma once
#include "noncopyable.h"

#include <functional>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <string>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads) {numThreads_ = numThreads;}
    
    void start(const ThreadInitCallback& cb = ThreadInitCallback());
    
    //工作在多线程中，baseLoo_默认以轮询的方式分配channel给subloop
    EventLoop* getNextLoop();

    std::vector<EventLoop*> getAllLoops();
    
    bool isStarted() const {return started_;}
    const std::string& getName() const {return name_;}
private:
    EventLoop* baseLoop_;
    std::string name_;
    int numThreads_;
    bool started_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};