#pragma once
#include "noncopyable.h"
#include <vector>
#include <unordered_map>
#include "Timestamp.h"

class EventLoop;
class Channel;

//muduo中多路分发器的核心IO复用模块
class Poller : noncopyable
{
public:
    using ChannelList=std::vector<Channel*>;

    Poller(EventLoop* loop);
    virtual ~Poller() = default;

    //给所有IO复用提供统一接口
    virtual Timestamp poll(int timeoutMs,ChannelList* activeChannels)=0;
    virtual void updateChannel(Channel* channel)=0;
    virtual void removeChannel(Channel* channel)=0;

    //判断channel是否在poller中注册
    bool hasChannel(Channel* channel) const;

    //EventLoop可以通过改接口获取默认的IO复用的具体实现
    static Poller* newDefaultPoller(EventLoop* loop);
protected:
    using ChannelMap=std::unordered_map<int,Channel*>;
    ChannelMap channels_;
private:
    EventLoop* ownerLoop_;
};