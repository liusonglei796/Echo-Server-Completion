#include <cstdio>
#include <cstdlib>
#include<sys/socket.h>
#include<netinet/in.h>
#include<cstring>
#include <sys/types.h>
#include<unistd.h>
#include <iostream>
#include <arpa/inet.h>

#define PORT 8081
#define LEN 1024
int main(int argc,const char *argv[]){
int server_fd,client_fd;
char buffer [LEN];
server_fd=socket(AF_INET, SOCK_STREAM, 0);
if(server_fd==-1){
perror("wrong");
close(server_fd);
exit(EXIT_FAILURE);
}
//IPV4地址结构体
struct sockaddr_in server_addr,client_addr;
memset(&server_addr, 0, sizeof(server_addr));
//设置端口号
server_addr.sin_port=htons(PORT);
//设置服务器监听的IP地址sin_addr也是一个结构体
server_addr.sin_addr.s_addr=INADDR_ANY;
//设置地址族
server_addr.sin_family=AF_INET;
if(bind(server_fd, (const struct sockaddr*)&server_addr, sizeof(server_addr))<0){
    perror("wrong");
    close(server_fd);
    exit(EXIT_FAILURE);
};
if(listen(server_fd, 5)<0){
        perror("wrong");
    close(server_fd);
    exit(EXIT_FAILURE);
};
socklen_t len=sizeof(client_addr);
client_fd=accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&len);
if(client_fd==-1){
    std::cerr<<"error"<<std::endl;
    exit(EXIT_FAILURE);
}
ssize_t read_bytes;
while((read_bytes=read(client_fd, buffer,LEN-1 ))>0){;
buffer[read_bytes]='\0'; 
    std::cout << "Received from client: " << buffer << std::endl;
ssize_t write_bytes=write(client_fd, buffer, read_bytes);
if(write_bytes<0){
     std::cerr << "Failed to write to socket" << std::endl;
            break;
}
}
if(read_bytes==0){
std::cout<<"disconnected"<<std::endl;
}
else if(read_bytes==-1){
    std::cerr<<"error"<<std::endl;
}
close(client_fd);
close(server_fd);
    return 0;
}