#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <string>
#include <cstring>
#include <thread>
#include <sys/epoll.h>

#include "toolFun.h"
#include "EpollServer.h"

#define Demo 0
#define SELECT 0
#define EPOLL 0
#define IO_USING 0
#define USE_SERVER 1

#if USE_SERVER
#else
#define BUFFER_LENGTH 8196
#endif

// 函数声明
std::string generateHttpResponse(const std::string &httpRequest)
{
    // 输出解析的http请求

    if (httpRequest.find("GET / HTTP/1.1") != std::string::npos)
    {
        // 处理根路径的HTTP请求
        std::ifstream file("/home/Pys/Cpp/SeverHttp/build/src/Homepage.html");
        if (!file.is_open())
        {
            return "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
        }

        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: text/html\r\n";

        // 获取文件大小
        file.seekg(0, std::ios::end);
        int fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        response << "Content-Length: " + std::to_string(fileSize) + "\r\n";

        response << "\r\n";
        response << file.rdbuf();

        file.close();
        return response.str();
    }
    else if (httpRequest.find("GET /Second.html HTTP/1.1") != std::string::npos)
    {
        // 处理 demo2.html 的HTTP请求
        std::ifstream file("/home/Pys/Cpp/SeverHttp/build/src/Second.html");
        if (!file.is_open())
        {
            return "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
        }

        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: text/html\r\n";

        // 获取文件大小
        file.seekg(0, std::ios::end);
        int fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        response << "Content-Length: " + std::to_string(fileSize) + "\r\n";

        response << "\r\n";
        response << file.rdbuf();

        file.close();
        return response.str();
    }
    else if (httpRequest.find("GET /a.png HTTP/1.1") != std::string::npos)
    {
        // 处理图片的HTTP请求
        std::ifstream imageFile("/home/Pys/Cpp/SeverHttp/build/src/a.png", std::ios::binary);
        if (!imageFile.is_open())
        {
            return "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
        }

        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: image/png\r\n"; // 设置正确的Content-Type

        // 获取文件大小
        imageFile.seekg(0, std::ios::end);
        int fileSize = imageFile.tellg();
        imageFile.seekg(0, std::ios::beg);

        response << "Content-Length: " + std::to_string(fileSize) + "\r\n";
        response << "\r\n";

        // 读取并发送图片数据
        response << imageFile.rdbuf();
        imageFile.close();
        return response.str();
    }
    else if (httpRequest.find("GET /iphone.mp4 HTTP/1.1") != std::string::npos)
    {
        std::ifstream videoFile("/home/Pys/Cpp/SeverHttp/build/src/iphone.mp4", std::ios::binary);
        if (!videoFile.is_open())
        {
            return "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
        }

        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: video/mp4\r\n";

        // 获取文件大小
        videoFile.seekg(0, std::ios::end);
        int fileSize = videoFile.tellg();
        videoFile.seekg(0, std::ios::beg);

        response << "Content-Length: " + std::to_string(fileSize) + "\r\n";
        response << "\r\n";

        // 读取并发送视频数据
        response << videoFile.rdbuf();

        videoFile.close();
        return response.str();
    }
    else if (httpRequest.find("GET /q.mp4 HTTP/1.1") != std::string::npos)
    {
        std::ifstream videoFile("/home/Pys/Cpp/SeverHttp/build/src/q.mp4", std::ios::binary);
        if (!videoFile.is_open())
        {
            return "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
        }

        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: video/mp4\r\n";

        // 获取文件大小
        videoFile.seekg(0, std::ios::end);
        int fileSize = videoFile.tellg();
        videoFile.seekg(0, std::ios::beg);

        response << "Content-Length: " + std::to_string(fileSize) + "\r\n";
        response << "\r\n";

        // 读取并发送视频数据
        response << videoFile.rdbuf();

        videoFile.close();
        return response.str();
    }
    else
    {
        // 处理其他路径的HTTP请求，可以根据需要添加更多逻辑
        return "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
    }
}

void routine(void *arg)
{

    int clientfd = *(int *)arg;

    while (1)
    {

        unsigned char buffer[BUFFER_LENGTH] = {0};
        int ret = recv(clientfd, buffer, BUFFER_LENGTH, 0);
        if (ret == 0)
        {
            close(clientfd);
            break;
        }
        printf("buffer : %s, ret: %d\n", buffer, ret);

        std::string httpRequest(reinterpret_cast<char *>(buffer), ret);
        // std::string httpResponse = generateHttpResponse(httpRequest);
        //  ret = send(clientfd, httpResponse.c_str(), httpResponse.length(), 0);

        int totalSent = 0;
        int dataLength = httpRequest.length();
        while (totalSent < dataLength)
        {
            ret = send(clientfd, httpRequest.c_str() + totalSent, dataLength - totalSent, 0);
            if (ret == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    // 资源暂时不可用，稍后重试
                    continue;
                }
                else
                {
                    // 发生其他错误，需要处理错误并退出循环
                    perror("send");
                    break;
                }
            }
            totalSent += ret;
        }

        // close(clientfd);
        // break;
    }
}

int main(int argc, char *argv[])
{
#if USE_SERVER
    CEpollServer server;
    server.InitServer();
    server.RunServer();
#else
    // 创建socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        return -1;
    }
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(80);
    // 绑定socket和端口
    if (-1 == bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)))
    {
        return -2;
    }
    // 监听socket
    listen(listenfd, 10);

#if Demo
    // 通过线程实现并发，但是没有对fd进行可读可写文件判断
    // 每一个连接对应一个线程
    // 最大并发量不高，且性能差
    while (1)
    {
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        std::cout << "等待接入" << std::endl;
        int clientfd = accept(listenfd, (struct sockaddr *)&client, &len);
        std::cout << "收到接入" << std::endl;

        std::thread fdThread(routine, &clientfd);
        fdThread.detach();
    }
#endif

#if SELECT

    int flags = fcntl(listenfd, F_GETFL, 0);
    fcntl(listenfd, F_SETFL, flags | O_NONBLOCK);

    // 通过系统接口select实现并发，并发量最大在1000左右
    fd_set rfds, wfds, rset, wset;
    FD_ZERO(&rfds);
    FD_SET(listenfd, &rfds);

    FD_ZERO(&wfds);

    int maxfd = listenfd;

    unsigned char buffer[BUFFER_LENGTH] = {0};
    int ret = 0;

    std::string httpResponse;
    while (true)
    {
        rset = rfds;
        wset = wfds;

        int nready = select(maxfd + 1, &rset, &wset, nullptr, nullptr);
        if (FD_ISSET(listenfd, &rset))
        {
            std::cout << "listenfd is react" << std::endl;
            struct sockaddr_in client;
            socklen_t len = sizeof(client);
            int clientfd = accept(listenfd, (struct sockaddr *)&client, &len);

            int flags = fcntl(clientfd, F_GETFL, 0);
            fcntl(clientfd, F_SETFL, flags | O_NONBLOCK);

            FD_SET(clientfd, &rfds);
            if (clientfd > maxfd)
            {
                maxfd = clientfd;
            }
        }
        for (int i = listenfd + 1; i <= maxfd; i++)
        {
            if (FD_ISSET(i, &rset))
            {
                ret = recv(i, buffer, BUFFER_LENGTH, 0);
                if (ret == 0)
                {
                    close(i);
                    std::cout << "close client connect" << std::endl;
                    FD_CLR(i, &rfds);
                }
                else if (ret > 0)
                {
                    printf("buffer : %s, ret: %d\n", buffer, ret);
                    std::string httpRequest(reinterpret_cast<char *>(buffer), ret);
                    httpResponse = generateHttpResponse(httpRequest);
                    std::cout << "收到请求：+++++++++++++++" << httpRequest << std::endl;
                    FD_SET(i, &wfds);
                }
            }
            else if (FD_ISSET(i, &wset))
            {
                std::cout << "send client buffer" << std::endl;
                // std::cout<<httpResponse<<std::endl;
                // ret=send(i,httpResponse.c_str(),httpResponse.length(),0);

                int totalSent = 0;
                int dataLength = httpResponse.length();

                std::cout << "发送出去的数据：+++++++++++++++" << httpResponse << std::endl;
                while (totalSent < dataLength)
                {
                    ret = send(i, httpResponse.c_str() + totalSent, dataLength - totalSent, 0);
                    if (ret == -1)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                        {
                            // 资源暂时不可用，稍后重试
                            continue;
                        }
                        else
                        {
                            // 发生其他错误，需要处理错误并退出循环
                            perror("send");
                            break;
                        }
                    }
                    totalSent += ret;
                }

                FD_CLR(i, &wfds);
                close(i);
                std::cout << "close client connect" << std::endl;
                FD_CLR(i, &rfds);
            }
        }
    }
#endif

#if EPOLL
#define EVENTS_LENGTH 128
    char epoll_buffer[BUFFER_LENGTH] = {0};
    int epoll_ret = 0;
    std::string epoll_httpResponse;

    int flags = fcntl(listenfd, F_GETFL, 0);
    fcntl(listenfd, F_SETFL, flags | O_NONBLOCK);

    int epfd = epoll_create(1);
    struct epoll_event ev, events[EVENTS_LENGTH];

    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);

    while (true)
    {
        int nReady = epoll_wait(epfd, events, EVENTS_LENGTH, 1000);
        for (int i = 0; i < nReady; i++)
        {
            int clientfd = events[i].data.fd;

            if (listenfd == clientfd)
            {
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int connfd = accept(listenfd, (struct sockaddr *)&client, &len);
                if (connfd == -1)
                    break;
                std::cout << "accept:" << connfd << std::endl;

                int flags = fcntl(connfd, F_GETFL, 0);
                fcntl(connfd, F_SETFL, flags | O_NONBLOCK);

                ev.events = EPOLLIN | EPOLLET; // 使用edge-trigger
                ev.data.fd = connfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
            }
            else if (events[i].events & EPOLLIN)
            {
                std::string httpRequest; // 用于累积完整的 HTTP 请求
                while (true)
                {
                    epoll_ret = recv(clientfd, epoll_buffer, BUFFER_LENGTH, 0);
                    if (epoll_ret > 0)
                    {
                        std::string receivedData(epoll_buffer, epoll_ret);
                        httpRequest += receivedData;
                    }
                    else if (epoll_ret == 0)
                    {
                        // 客户端关闭连接的处理
                        close(clientfd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, clientfd, nullptr);
                        std::cout << "客户但断开关闭连接:" << clientfd << std::endl;
                        break; // 退出循环
                    }
                    else if (epoll_ret == -1 && errno == EAGAIN)
                    {
                        // 暂时没有更多数据可读，退出循环等待下一次 EPOLLIN 事件
                        break;
                    }
                    else
                    {
                        // 处理接收错误
                        perror("recv");
                        close(clientfd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, clientfd, nullptr);
                        break; // 退出循环
                    }
                }

                if (!httpRequest.empty())
                {
                    epoll_httpResponse = handleHttpRequest(httpRequest);

                    ev.events = EPOLLOUT;
                    ev.data.fd = clientfd;
                    epoll_ctl(epfd, EPOLL_CTL_MOD, clientfd, &ev);
                }
            }
            else if (events[i].events & EPOLLOUT)
            {
                int totalSent = 0;
                int dataLength = epoll_httpResponse.length();

                while (totalSent < dataLength)
                {
                    int bytesToSend = std::min(dataLength - totalSent, BUFFER_LENGTH); // 每次发送的数据大小

                    epoll_ret = send(clientfd, epoll_httpResponse.c_str() + totalSent, bytesToSend, MSG_NOSIGNAL);
                    if (epoll_ret == -1)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                        {
                            // 资源暂时不可用，稍后重试
                            continue;
                        }
                        else if (errno == EPIPE)
                        {
                            close(clientfd);
                            epoll_ctl(epfd, EPOLL_CTL_DEL, clientfd, nullptr);
                            std::cout << "浏览器主动关闭连接:" << clientfd << std::endl;
                        }
                        else
                        {
                            // 发生其他错误，需要处理错误并退出循环
                            perror("send");
                            break;
                        }
                    }

                    totalSent += epoll_ret;
                }

                // 因为是Http所以处理完数据后关闭连接
                close(clientfd);
                epoll_ctl(epfd, EPOLL_CTL_DEL, clientfd, nullptr);
                std::cout << "关闭连接:" << clientfd << std::endl;
            }
        }
    }
#endif
#endif
    return 0;
}
