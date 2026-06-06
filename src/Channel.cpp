#include "Channel.h"
#include "EventLoop.h"
#include <sys/epoll.h>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN|EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop,int fd)
    :loop_(loop)
    ,fd_(fd)
    ,events_(0)
    ,revents_(0)
    ,index_(-1)
    ,tied_(false)
{
}

Channel::~Channel()
{
}

void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_=obj;
    tied_ = true;
}

//当改变channel所表示的fd的events事件后，需要更新epoll
void Channel::update()
{
    //通过channel所属的eventloop更新channel的events事件
    loop_->updateChannel(this);
}

//在所属的eventloop中删除channel
void Channel::remove()
{
    loop_->removeChannel(this);
}

//fd得到poller通知后，处理时间的函数
void Channel::handleEvent(Timestamp receiveTime)
{
    if(tied_)
    {
        std::shared_ptr<void> guard=tie_.lock();
        if(guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    if((revents_ & EPOLLHUP)&&!(revents_ & EPOLLIN))
    {
        if(closeCallback_)
        {
            closeCallback_();
        }
    }

    if(revents_ & EPOLLERR)
    {
        if(errorCallback_)
        {
            errorCallback_();
        }
    }

    if(revents_ & (EPOLLIN|EPOLLPRI))
    {
        if(readEventCallback_)
        {
            readEventCallback_(receiveTime);
        }
    }

    if(revents_ & EPOLLOUT)
    {
        if(writeCallback_)
        {
            writeCallback_();
        }
    }
}
