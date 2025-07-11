#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <array>
#include <csignal>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include "util.h"

#define MAX_EVENTS 1024   // 定义epoll最大事件数量
#define READ_BUFFER 1024  // 定义读取缓冲区大小

// 设置文件描述符为非阻塞模式
void setnonblocking(int fd){
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);  // 获取当前标志位并添加非阻塞标志
}

volatile bool running = true;
void handle_sigint(int) {
    running = false;
}

int main() {
    std::signal(SIGINT, handle_sigint);
    int sockfd = -1, epfd = -1;
    try {  // 开始异常处理块
        // 创建TCP socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);  // 创建IPv4 TCP socket
        errif(sockfd == -1, "socket create error");    // 检查socket创建是否成功

        // 初始化服务器地址结构体
        struct sockaddr_in serv_addr{};  // 使用C++11统一初始化语法，自动清零
        serv_addr.sin_family = AF_INET;  // 设置地址族为IPv4
        serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // 设置IP地址为本地回环地址
        serv_addr.sin_port = htons(8888);  // 设置端口号为8888，转换为网络字节序

        // 绑定socket到指定地址和端口
        errif(bind(sockfd, reinterpret_cast<sockaddr*>(&serv_addr), sizeof(serv_addr)) == -1, "socket bind error");

        // 开始监听连接
        errif(listen(sockfd, SOMAXCONN) == -1, "socket listen error");  // SOMAXCONN为最大连接队列长度

        // 创建epoll实例
        epfd = epoll_create1(0);  // 创建epoll文件描述符
        errif(epfd == -1, "epoll create error");

        // 初始化epoll事件数组和事件结构体
        std::array<epoll_event, MAX_EVENTS> events{};  // 事件数组和单个事件结构体
        struct epoll_event event{};
        event.data.fd = sockfd;  // 设置事件关联的文件描述符
        event.events = EPOLLIN | EPOLLET;  // 设置事件类型：可读事件 + 边缘触发模式
        setnonblocking(sockfd);  // 设置socket为非阻塞模式
        errif(epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &event) == -1, "epoll_ctl add listen fd error");  // 错误处理

        // 主事件循环
        while(running){
            // 等待epoll事件
            int nfds = epoll_wait(epfd, events.data(), MAX_EVENTS, -1);  // 阻塞等待事件，-1表示无限等待
            if(nfds == -1) {
                if(errno == EINTR) continue; // 被信号中断，继续等待
                errif(true, "epoll wait error");
            }
            // 处理所有就绪的事件
            for(int i = 0; i < nfds; ++i){
                int curfd = events[i].data.fd;
                uint32_t curevents = events[i].events;
                if(curfd == sockfd){        // 新客户端连接事件
                    // 边缘触发下，accept 需循环直到 EAGAIN
                    while(true) {
                        struct sockaddr_in clnt_addr{};
                        socklen_t clnt_addr_len = sizeof(clnt_addr);
                        int clnt_sockfd = accept(sockfd, reinterpret_cast<sockaddr*>(&clnt_addr), &clnt_addr_len);
                        if(clnt_sockfd == -1) {
                            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                                // 已无新连接
                                break;
                            } else if(errno == EINTR) {
                                continue;
                            } else {
                                perror("socket accept error");
                                break;
                            }
                        }
                        // 打印新客户端信息
                        printf("new client fd %d! IP: %s Port: %d\n", clnt_sockfd, inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));
                        // 将新客户端socket添加到epoll监听列表
                        event = {};
                        event.data.fd = clnt_sockfd;
                        event.events = EPOLLIN | EPOLLET;
                        setnonblocking(clnt_sockfd);
                        errif(epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sockfd, &event) == -1, "epoll_ctl add client fd error");
                    }
                } else if(curevents & (EPOLLERR | EPOLLHUP)) {
                    // 处理错误和挂起事件
                    fprintf(stderr, "epoll error/hup on fd %d, closing\n", curfd);
                    close(curfd);
                } else if(curevents & EPOLLIN){      // 客户端数据可读事件
                    char buf[READ_BUFFER];  // 读取缓冲区
                    while(true){    // 由于使用非阻塞IO，需要循环读取直到数据全部读取完毕
                        std::memset(buf, 0, sizeof(buf));  // 清零缓冲区
                        ssize_t bytes_read = read(curfd, buf, sizeof(buf));  // 读取数据
                        if(bytes_read > 0){  // 成功读取到数据
                            printf("message from client fd %d: %s\n", curfd, buf);  // 打印接收到的消息
                            write(curfd, buf, bytes_read);  // 回显数据给客户端
                        } else if(bytes_read == -1 && errno == EINTR){  // 被信号中断，继续读取
                            printf("continue reading");
                            continue;
                        } else if(bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))){  // 非阻塞IO，数据全部读取完毕
                            printf("finish reading once, errno: %d\n", errno);
                            break;  // 退出读取循环
                        } else if(bytes_read == 0){  // 客户端断开连接（EOF）
                            printf("EOF, client fd %d disconnected\n", curfd);
                            close(curfd);   // 关闭客户端socket，会自动从epoll树上移除
                            break;  // 退出读取循环
                        } else { // 其他错误
                            perror("read error");
                            close(curfd);
                            break;
                        }
                    }
                } else{         // 其他事件类型，后续版本实现
                    printf("something else happened\n");
                }
            }
        }
    } catch(const SyscallException& e) {  // 捕获系统调用异常
        std::fprintf(stderr, "[SyscallException] %s\n", e.what());
        // 输出异常信息到标准错误
        if(sockfd != -1) close(sockfd);
        if(epfd != -1) close(epfd);
        return -1;  // 返回错误码
    }
    // 退出前关闭所有fd
    if(sockfd != -1) close(sockfd);
    if(epfd != -1) close(epfd);
    return 0;  // 正常退出
}

