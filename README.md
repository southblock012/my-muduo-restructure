# my_muduo — 基于 Reactor 模型的多线程 C++ 网络库

## 项目简介

my_muduo 是一个基于 **one loop per thread** 架构的多线程 C++ 网络库，仿照 muduo 网络库实现。该项目使用 C++11 标准编写，底层基于 Linux epoll 实现高性能 IO 多路复用，适用于构建高并发 TCP 服务器。

核心设计思想：**主从 Reactor 模型** — 一个 main-loop 负责接受新连接，多个 sub-loop 负责处理已建立连接的 IO 事件。通过将连接接受与 IO 处理分离，充分利用多核 CPU，实现高性能并行处理。

---

## 核心技术栈与亮点

### 技术栈

| 技术 | 用途 |
|------|------|
| **C++11** | 语言标准（智能指针、std::function、原子操作等） |
| **Linux epoll** | IO 多路复用（边缘触发/水平触发） |
| **eventfd** | 跨线程事件通知与唤醒 |
| **CMake** | 构建系统 |
| **多线程编程** | 线程池、锁、条件变量、无锁队列技巧 |

### 核心亮点

1.  **主从 Reactor 多线程模型**
    - Acceptor 运行在 main-loop，只负责 accept 新连接
    - 通过轮询算法将新连接均匀分发到多个 sub-loop
    - sub-loop 各自独立运行 EventLoop，互不干扰，并行处理 IO

2.  **Channel 双层回调设计**
    - **底层**：Channel 注册 `EPollPoller` 返回的原始 epoll 事件，绑定框架内部的 `handleRead/handleWrite/handleClose`
    - **上层**：TcpConnection 对外暴露 `MessageCallback/ConnectionCallback` 等业务回调
    - 用户只需关注业务逻辑，无需接触 epoll 细节

3.  **跨线程任务投递与唤醒**
    - 通过 `eventfd` 实现跨线程安全唤醒
    - `queueInLoop` / `runInLoop` 接口支持从任意线程向指定 EventLoop 投递任务
    - `doPendingFunctors` 采用 **lock-free swap** 技巧：锁内交换队列，锁外执行回调，最小化锁持有时间

4.  **EPollPoller 三级状态机**
    - 每个 Channel 在 Poller 中维护 `kNew → kAdded → kDeleted` 三种状态
    - `updateChannel` 根据状态自动选择 `epoll_ctl` 的 ADD/MOD/DEL 操作
    - `epoll_wait` 返回的事件列表支持动态扩容

5.  **TcpConnection 四状态机 + 优雅关闭**
    - 状态流转：`kConnecting → kConnected → kDisconnecting → kDisconnected`
    - 调用 `shutdown()` 后，等待 `outputBuffer_` 数据全部发送完才真正调用 `::shutdown(SHUT_WR)` 发送 FIN
    - `handleWrite` 中内置补位逻辑：数据意外发完时自动补执行关闭

6.  **tie 机制 — 多线程对象生命周期保护**
    - Channel 通过 `weak_ptr<TcpConnection>` 绑定的方式，在执行回调前先检查 TcpConnection 是否已被销毁
    - 配合 `enable_shared_from_this` 和 `shared_ptr` 引用计数管理，从根本上解决多线程环境下回调时对象已析构的竞态问题

7.  **非阻塞 IO 缓冲区**
    - 读写分离设计（`readerIndex_` / `writerIndex_`）
    - `readFd` 使用 `readv` + 栈上 64KB 缓冲区的零拷贝优化策略，减少系统调用和内存分配
    - 支持自动扩容

8.  **HighWaterMark 水位背压控制**
    - 发送缓冲区超过用户设定的水位线阈值时触发回调
    - 可用于检测慢客户端、降低发送速率或强制断开连接，防止内存被打满

---

## 目录结构

```
my_muduo/
├── CMakeLists.txt             # CMake 构建配置，编译为动态库 libmy_muduo.so
├── autobuild.sh               # 自动构建脚本（Linux）
│
├── include/                   # 头文件（公共接口）
│   ├── Acceptor.h             # 接受器，封装 listenfd 的 accept 逻辑
│   ├── Buffer.h               # 非阻塞 IO 缓冲区
│   ├── Callbacks.h            # 回调类型定义（MessageCallback 等）
│   ├── Channel.h              # IO 事件通道，封装 fd 及事件回调
│   ├── CurrentThread.h        # 当前线程 tid 缓存
│   ├── EPollPoller.h          # epoll IO 复用封装
│   ├── EventLoop.h            # 事件循环（Reactor 的核心）
│   ├── EventLoopThread.h      # 一个 EventLoop + 一个线程的封装
│   ├── EventLoopThreadPool.h  # EventLoop 线程池
│   ├── InetAddress.h          # 网络地址封装
│   ├── Poller.h               # IO 复用抽象基类
│   ├── Socket.h               # Socket 封装（bind/listen/accept/setsockopt）
│   ├── TcpConnection.h        # TCP 连接管理
│   ├── TcpServer.h            # TCP 服务器（供用户使用的入口类）
│   ├── Thread.h               # 线程封装
│   ├── Timestamp.h            # 时间戳
│   └── noncopyable.h          # 不可拷贝基类
│
├── src/                       # 源文件
│   ├── Acceptor.cpp
│   ├── Buffer.cpp
│   ├── Channel.cpp
│   ├── CurrentThread.cpp
│   ├── DefaultPoller.cpp      # 默认 Poller 选择（返回 EPollPoller）
│   ├── EPollPoller.cpp
│   ├── EventLoop.cpp
│   ├── EventLoopThread.cpp
│   ├── EventLoopThreadPool.cpp
│   ├── InetAddress.cpp
│   ├── Poller.cpp
│   ├── Socket.cpp
│   ├── TcpConnection.cpp
│   ├── TcpServer.cpp
│   ├── Thread.cpp
│   └── Timestamp.cpp
│
├── example/                   # 示例程序
│   ├── echosever.cpp          # Echo 服务器（收到什么回什么）
│   └── Makefile
│
├── lib/                       # 编译产物（构建后生成）
└── README.md
```

---

## 环境依赖

### 操作系统

- **Linux**（推荐 Ubuntu 18.04+ / CentOS 7+）
- 依赖 Linux 内核特性：`epoll`、`eventfd`

> 注意：该项目使用了 POSIX 系统调用（`epoll`、`eventfd`、`pthread` 等），**不支持 Windows 或 macOS**。

### 编译器

- **GCC 4.8.1+**（需支持 C++11）
- 建议 GCC 7+ 以获得更好的 C++11 支持

### 构建工具

- **CMake 3.10+**
- **make**

### 依赖库

- **libpthread**（线程支持，系统自带）

---

## 如何编译与安装

### 编译动态库

```bash
# 方法一：使用自动构建脚本
cd my_muduo
chmod +x autobuild.sh
./autobuild.sh

# 方法二：使用 CMake 手动构建
cd my_muduo
mkdir build && cd build
cmake ..
make -j$(nproc)
```

编译完成后，动态库 `libmy_muduo.so` 会生成在项目根目录的 `lib/` 文件夹中。

### 编译示例程序

```bash
cd my_muduo/example

# 方法一：直接使用 Makefile
make

# 方法二：手动编译（指定库路径和头文件路径）
g++ -o echosever echosever.cpp -lmy_muduo -lpthread -g -I../include -L../lib
```

运行前需要将动态库路径加入加载路径：

```bash
export LD_LIBRARY_PATH=../lib:$LD_LIBRARY_PATH
./echosever
```

### 在自己的项目中使用

```cmake
# CMakeLists.txt
include_directories(/path/to/my_muduo/include)
link_directories(/path/to/my_muduo/lib)
target_link_libraries(your_target my_muduo pthread)
```

### 快速测试

运行示例 Echo 服务器后，可用 `telnet` 或 `nc` 测试：

```bash
# 终端 1：启动服务器
cd my_muduo/example
export LD_LIBRARY_PATH=../lib:$LD_LIBRARY_PATH
./echosever

# 终端 2：连接测试
telnet 127.0.0.1 8080
# 输入任意字符，服务器会原样返回并关闭连接
```
