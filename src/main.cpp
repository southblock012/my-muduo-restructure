#include "EventLoop.h"
#include "Channel.h"
#include "Socket.h"
#include "InetAddress.h"
#include <iostream>
#include <unistd.h>
#include <memory>
#include <string.h>

const int PORT = 8080;
const int BUFFER_SIZE = 1024;

// 简单的 Echo 连接处理
class EchoConnection
{
public:
    EchoConnection(EventLoop* loop, int sockfd)
        : loop_(loop)
        , sockfd_(sockfd)
        , channel_(new Channel(loop, sockfd))
    {
        channel_->setReadCallback(std::bind(&EchoConnection::handleRead, this, std::placeholders::_1));
        channel_->enableReading();
    }

    ~EchoConnection()
    {
        close(sockfd_);
    }

private:
    void handleRead(Timestamp receiveTime)
    {
        char buf[BUFFER_SIZE];
        memset(buf, 0, BUFFER_SIZE);
        ssize_t n = read(sockfd_, buf, BUFFER_SIZE - 1);
        
        if (n > 0)
        {
            std::cout << "Received: " << buf << std::endl;
            write(sockfd_, buf, n);  // 回显
        }
        else if (n == 0)
        {
            std::cout << "Connection closed" << std::endl;
            channel_->disableAll();
            channel_->remove();
            delete this;  // 自删除（简单处理）
        }
        else
        {
            perror("read error");
        }
    }

    EventLoop* loop_;
    int sockfd_;
    std::unique_ptr<Channel> channel_;
};

// 简单的接受器
class Acceptor
{
public:
    Acceptor(EventLoop* loop, const InetAddress& listenAddr)
        : loop_(loop)
        , listenSocket_(new Socket(::socket(AF_INET, SOCK_STREAM, 0)))
        , acceptChannel_(new Channel(loop, listenSocket_->fd()))
    {
        listenSocket_->setReuseAddr(true);
        listenSocket_->bindAddress(listenAddr);
        listenSocket_->listen();
        
        acceptChannel_->setReadCallback(std::bind(&Acceptor::handleRead, this));
        acceptChannel_->enableReading();
        
        std::cout << "EchoServer listening on " << listenAddr.toIpPort() << std::endl;
    }

private:
    void handleRead()
    {
        InetAddress peerAddr(0);
        int connfd = listenSocket_->accept(&peerAddr);
        
        if (connfd >= 0)
        {
            std::cout << "New connection from " << peerAddr.toIpPort() << std::endl;
            new EchoConnection(loop_, connfd);
        }
    }

    EventLoop* loop_;
    std::unique_ptr<Socket> listenSocket_;
    std::unique_ptr<Channel> acceptChannel_;
};

int main()
{
    EventLoop loop;
    InetAddress listenAddr(PORT);
    Acceptor acceptor(&loop, listenAddr);
    loop.loop();
    return 0;
}
