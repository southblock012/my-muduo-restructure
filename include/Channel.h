#pragma once
#include "noncopyable.h"
#include <functional>
#include <memory>

class EventLoop;
class TcpServer;
class Timestamp;

//封装了sockfd和感兴趣的event类型
class Channel : public noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop* loop,int fd);
    ~Channel();
    
    //处理事件，调用相应的回调函数
    void handleEvent(Timestamp receiveTime);

    //设置回调函数对象
    void setReadCallback(ReadEventCallback cb){readEventCallback_ =std::move(cb);}
    void setWriteCallback(EventCallback cb){writeCallback_ =std::move(cb);}
    void setCloseCallback(EventCallback cb){closeCallback_ =std::move(cb);}
    void setErrorCallback(EventCallback cb){errorCallback_ =std::move(cb);}

    //防止当channel被手动remove掉，channel还在执行回调操作
    void tie(const std::shared_ptr<void>&);

    int fd() const{return fd_;}
    int events() const{return events_;}
    int revents() const{return revents_;}
    void set_revents(int revt){revents_ = revt;}

    //设置fd相应的状态
    void enableReading(){events_ |= kReadEvent;update();}
    void disableReading(){events_ &= ~kReadEvent;update();}
    void enableWriting(){events_ |= kWriteEvent;update();}
    void disableWriting(){events_ &= ~kWriteEvent;update();}
    void disableAll(){events_ = kNoneEvent;update();}

    //返回fd当前事件状态
    bool isNoneEvent() const{return events_ == kNoneEvent;}
    bool isReading() const{return events_ & kReadEvent;}
    bool isWriting() const{return events_ & kWriteEvent;}

    int index() const{return index_;}
    void set_index(int idx){index_ = idx;}

    //one loop per thread
    EventLoop* ownerLoop() const{return loop_;}
    void remove();

private:
    
    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop *loop_;//事件循环
    const int fd_;//sockfd
    int events_;//感兴趣的event类型
    int revents_;//实际发生的event类型
    int index_;//在EventLoop中的索引
    
    std::weak_ptr<void> tie_;
    bool tied_;

    //因为Channel通道里面能获知fd最终发生的具体的事件类型，所以这里需要负责调用每个事件类型的回调函数
    ReadEventCallback readEventCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};
