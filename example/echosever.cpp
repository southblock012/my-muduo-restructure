#include <my_muduo/TcpServer.h>

#include <string>
#include <iostream>

class EchoServer
{
public:
    EchoServer(EventLoop *loop,const InetAddress &addr,const std::string &name)
        :loop_(loop)
        ,server_(loop,addr,name)
    {
        //注册回调函数
        server_.setConnectionCallback(std::bind(&EchoServer::onConnection,this,std::placeholders::_1));
        server_.setMessageCallback(std::bind(&EchoServer::onMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));

        //设置合适的loop线程数量
        server_.setThreadNum(3);
    }
    // 开启服务器监听
    void start()
    {
        server_.start();
    }
private:
    //连接建立或者断开的回调
    void onConnection(const TcpConnectionPtr &conn)
    {
        if(conn->connected())
        {
            std::cout<<"连接建立"<<std::endl;
        }
        else
        {
            std::cout<<"连接断开"<<std::endl;
        }
    }

    //可读写事件回调
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
    {
        std::string msg = buf->retrieveAllAsString();
        conn->send(msg);
        conn->shutdown();// 关闭连接
    }

    EventLoop *loop_;
    TcpServer server_;
};

int main()
{
    EventLoop loop;
    InetAddress addr(8080,"0.0.0.0");
    EchoServer server(&loop,addr,"EchoServer-01");
    server.start();
    loop.loop();
    return 0;
}

