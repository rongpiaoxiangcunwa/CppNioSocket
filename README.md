# CppNioSocket
CppNioSocket 是使用C++ BOOST实现的基于NIO和事件驱动的客户/服务器编程架构。
整体架构采用了reactor模型，采用epoll机制：
    通过Reactor模型基于多路复用器接收并处理用户请求，内部实现了两个线程池，boss线程池和work线程池，其中boss线程池的线程负责处理请求的accept事件，当接收到accept事件的请求时，把对应的socket封装到一个 connection 中，并交给work线程池，其中work线程池负责请求的read和write事件。


# 特点：
异步非阻塞；
基于事件驱动；
性能高；
支持多种线程模型(单线程模型，多线程模型，主从线程模型)；    
具有高可定制性和扩展性；

# 开发环境
    语言：C++
    依赖库：boost_1_68
    平台：linux
    
# 性能测试：
    测试服务端采用了主从线程模型（2个acceptor线程， 2个work线程）。客户端模拟2万个TCP连接，QPS为2万。服务端CPU使用率为 68%, 内存占用为 168Mb
    
    

# 快速开始：
    clone该工程到你本地的liux环境
    
