#include <cstdlib>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <iostream>
#include <string> // 推荐使用 std::string 和 std::getline

#define SERVER_IP "127.0.0.1"
#define PORT 8081
#define BUFFER_SIZE 1024

void error_exit(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, const char* argv[]) {
    int sock_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE]; // 用于接收服务器回显的缓冲区
    std::string input_line;   // 使用 std::string 来处理用户输入，更安全方便

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        error_exit("socket creation failed");
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_port = htons(PORT);
    server_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        error_exit("inet_pton failed for server IP");
    }

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error_exit("connect failed");
    }

    std::cout << "Connected to server. You can start typing." << std::endl;
    std::cout << "Type 'exit' to quit." << std::endl;

    // 使用 std::getline 读取一整行到 std::string，这是推荐的做法
    while (std::getline(std::cin, input_line)) {
        if (input_line == "exit") {
            break;
        }
        if (input_line.empty()) { // 如果用户只按回车，则跳过
            continue;
        }

        // **修正点 1 & 2: 发送用户输入的实际内容(input_line)和其实际长度**
        // .c_str() 获取C风格字符串指针，.length() 获取其实际长度
        ssize_t written_bytes = write(sock_fd, input_line.c_str(), input_line.length());
        if (written_bytes < 0) {
            error_exit("Failed to write to socket");
            break;
        }

        // 从服务器读取回显消息
        ssize_t bytes_read = read(sock_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read < 0) {
            error_exit("Failed to read from socket");
            break;
        }
        if (bytes_read == 0) {
            std::cout << "Server closed the connection." << std::endl;
            break;
        }

        // **修正点 3: 手动在读取的数据末尾添加空终止符，确保它是一个有效的C字符串**
        buffer[bytes_read] = '\0';

        std::cout << "Server echo: " << buffer << std::endl;
    }

    close(sock_fd);
    std::cout << "Connection closed." << std::endl;
    return 0;
}