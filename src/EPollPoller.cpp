#include "EPollPoller.h"
#include "Channel.h"
#include <errno.h>
#include <unistd.h>
#include <cstring>

const int kNew=-1;
const int kAdded=1;
const int kDeleted=2;

EPollPoller::EPollPoller(EventLoop* loop)
    :Poller(loop)
    ,epollFd_(::epoll_create1(EPOLL_CLOEXEC))
    ,events_(kInitEventListSize) // vector<epoll_event>
{
}

EPollPoller::~EPollPoller()
{
    ::close(epollFd_);
}

Timestamp EPollPoller::poll(int timeoutMs,ChannelList* activeChannels)
{
    int numEvents=::epoll_wait(epollFd_,&*events_.begin(),static_cast<int>(events_.size()),timeoutMs);
    int saveErrno=errno;
    Timestamp now(Timestamp::Now());

    if(numEvents>0)
    {
        fillActiveChannels(numEvents,activeChannels);
        if(numEvents==events_.size())
        {
            events_.resize(events_.size()*2);
        }
    }
    else if(numEvents==0)
    {
       
    }
    else
    {
        if(saveErrno!=EINTR)
        {
           errno=saveErrno;
        }
    }
    return now;
}

void EPollPoller::updateChannel(Channel* channel)
{
    const int index_=channel->index();
    if(index_==kNew || index_==kDeleted)
    {   
        if(index_==kNew)
        {
            int fd=channel->fd();
            channels_[fd]=channel;
        }
        else // index == kDeleted
        {
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD,channel);
    }
    else//channel已经注册过了
    {
        int fd=channel->fd();
        if(channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL,channel);
        }
        else
        {
            update(EPOLL_CTL_MOD,channel);
        }
    }
}

void EPollPoller::removeChannel(Channel* channel)
{
    int fd=channel->fd();
    channels_.erase(fd);

    int index=channel->index();
    if(index==kAdded)
    {
        update(EPOLL_CTL_DEL,channel);
    }

    channel->set_index(kNew);
}

// 填写活跃的连接
void EPollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
    for (int i = 0; i < numEvents; ++i)
    {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel); // EventLoop就拿到了它的Poller给它返回的所有发生事件的channel列表了
    }
}

void EPollPoller::update(int operation, Channel* channel)
{
    epoll_event event;
    memset(&event,0,sizeof(event));
    event.events=channel->events();
    event.data.ptr=channel;
    int fd=channel->fd();

    if(::epoll_ctl(epollFd_,operation,fd,&event)<0)
    {
        if(operation==EPOLL_CTL_DEL)
        {

        }
        else
        {

        }
    }
}