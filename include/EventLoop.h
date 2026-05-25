#pragma once

#include <vector>
#include <functional>
#include <atomic>
#include <memory>
#include <mutex>

#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"

class Channel;
class Poller;

//事件循环类,主要包含Channel Poller等类
class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    //开启事件循环
    void loop();
    //退出事件循环
    void quit();

    Timestamp pollReturnTime() const { return pollReturnTime_; }

    //在当前loop中执行cb
    void runInLoop(Functor cb);
    //把cb放入队列中，唤醒loop所在的线程，执行cb
    void queueInLoop(Functor cb);

    //用来唤醒loop所在的线程
    void wakeup();

    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);

    //判断eventloop是否在loop所在的线程中执行
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
private:
    void handleRead();
    void doPendingFunctors();

    using ChannelList = std::vector<Channel*>;

    std::atomic_bool looping_;
    std::atomic_bool quit_;//标识退出事件循环

    const pid_t threadId_;//当前线程的ID

    Timestamp pollReturnTime_;//事件轮询器返回的时间戳
    std::unique_ptr<Poller> poller_;//事件轮询器
    
    int wakeupFd_;//唤醒fd,用于唤醒事件循环，通过轮询算法唤醒subloop
    std::unique_ptr<Channel> wakeupChannel_;
    
    ChannelList activeChannels_;
    Channel *currentActiveChannel_;
    
    std::atomic_bool callingPendingFunctors_;//标识当前loop是否有需要执行的回调操作
    std::vector<Functor> pendingFunctors_;//存储loop需要的回调操作
    std::mutex mutex_;//互斥锁，保护上面vector容器的线程安全操作
};