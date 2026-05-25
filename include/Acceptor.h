#pragma once

#include "noncopyable.h"
#include "InetAddress.h"
#include "Socket.h"
#include "Channel.h"

class InetAddress;

class Acceptor : noncopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;
    Acceptor(EventLoop* loop, const InetAddress& localaddr, bool reusePort = false);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb){newConnectionCallback_ = cb;}

    bool listening() const {return listening_;}
    void listen();
private:
    void handleRead();
    
    NewConnectionCallback newConnectionCallback_;
    bool listening_;
    EventLoop *loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
};