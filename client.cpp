#include <iostream>     // 包含C++标准输入输出流
#include <cstdio>       // 包含C风格输入输出函数
#include <cstring>      // 包含字符串操作函数
#include <sys/socket.h> // 包含socket相关函数和结构体
#include <arpa/inet.h>  // 包含网络地址转换函数
#include <unistd.h>     // 包含系统调用函数
#include <errno.h>      // 包含错误码定义
#include "util.h"       // 包含自定义工具函数

#define BUFFER_SIZE 1024  // 定义缓冲区大小

int main() {
    try {  // 开始异常处理块
        // 创建TCP socket
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);  // 创建IPv4 TCP socket
        errif(sockfd == -1, "socket create error");    // 检查socket创建是否成功

        // 初始化服务器地址结构体
        struct sockaddr_in serv_addr{};  // 使用C++11统一初始化语法，自动清零
        serv_addr.sin_family = AF_INET;  // 设置地址族为IPv4
        serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // 设置IP地址为本地回环地址
        serv_addr.sin_port = htons(8888);  // 设置端口号为8888，转换为网络字节序

        // 连接到服务器
        errif(connect(sockfd, reinterpret_cast<sockaddr*>(&serv_addr), sizeof(serv_addr)) == -1, "socket connect error");

        // 主通信循环
        bool should_exit = false;  // 退出标志
        while(!should_exit){
            char buf[BUFFER_SIZE];  // 通信缓冲区
            std::memset(buf, 0, sizeof(buf));  // 清零缓冲区
            scanf("%s", buf);  // 从标准输入读取用户输入

            // 发送数据到服务器
            ssize_t write_bytes = write(sockfd, buf, sizeof(buf));  // 发送整个缓冲区
            if(write_bytes == -1){  // 发送失败
                printf("socket already disconnected, can't write any more!\n");
                break;  // 退出循环
            }

            // 循环读取服务器响应，直到接收完所有数据
            while(true){
                std::memset(buf, 0, sizeof(buf));  // 清零缓冲区
                ssize_t read_bytes = read(sockfd, buf, sizeof(buf));  // 读取服务器响应

                if(read_bytes > 0){  // 成功读取到数据
                    printf("message from server: %s\n", buf);  // 打印服务器响应
                    // 如果读取的数据小于缓冲区大小，说明数据接收完毕
                    if(read_bytes < sizeof(buf)){
                        break;  // 退出读取循环
                    }
                }else if(read_bytes == 0){  // 服务器断开连接（EOF）
                    printf("server socket disconnected!\n");
                    should_exit = true;  // 设置退出标志
                    break;  // 退出读取循环
                }else if(read_bytes == -1){  // 读取出错
                    if(errno == EAGAIN || errno == EWOULDBLOCK){  // 非阻塞模式下，数据读完
                        break;  // 退出读取循环
                    }else{  // 其他错误
                        close(sockfd);  // 关闭socket
                        errif(true, "socket read error");  // 抛出读取错误异常
                    }
                }
            }
        }
        close(sockfd);  // 关闭客户端socket

    } catch(const SyscallException& e) {  // 捕获系统调用异常
        std::fprintf(stderr, "[SyscallException] %s\n", e.what());  // 输出异常信息到标准错误
        return -1;  // 返回错误码
    }
    return 0;  // 正常退出
}
