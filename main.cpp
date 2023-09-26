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
#define Demo 0
#define SELECT 0
#define EPOLL 1
#define IO_USING 0

#define BUFFER_LENGTH 8196

// 函数声明
std::string generateHttpResponse(const std::string &httpRequest)
{
    if (httpRequest.find("GET / HTTP/1.1") != std::string::npos)
    {
        // 处理根路径的HTTP请求
        std::ifstream file("src/Homepage.html");
        if (!file.is_open())
        {
            return "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
        }

        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: text/html\r\n";
        response << "\r\n";
        response << file.rdbuf();

        return response.str();
    }
    else if (httpRequest.find("GET /Second.html HTTP/1.1") != std::string::npos)
    {
        // 处理 demo2.html 的HTTP请求
        std::ifstream demo2File("src/Second.html");
        if (!demo2File.is_open())
        {
            return "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
        }

        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: text/html\r\n";
        response << "\r\n";
        response << demo2File.rdbuf();

        return response.str();
    }
    else if (httpRequest.find("GET /a.png HTTP/1.1") != std::string::npos)
    {
        // 处理图片的HTTP请求
        std::ifstream imageFile("src/a.png", std::ios::binary);
        if (!imageFile.is_open())
        {
            return "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
        }

        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: image/png\r\n"; // 设置正确的Content-Type
        response << "\r\n";

        // 读取并发送图片数据
        response << imageFile.rdbuf();

        return response.str();
    }
    else if (httpRequest.find("GET /iphone.mp4 HTTP/1.1") != std::string::npos)
    {
        // 处理图片的HTTP请求
        std::ifstream imageFile("src/iphone.mp4", std::ios::binary);
        if (!imageFile.is_open())
        {
            return "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
        }

        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: image/png\r\n"; // 设置正确的Content-Type
        response << "\r\n";

        // 读取并发送图片数据
        response << imageFile.rdbuf();

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
        std::string httpResponse = generateHttpResponse(httpRequest);
        ret = send(clientfd, httpResponse.c_str(), httpResponse.length(), 0);
    }
}

int main(int argc, char *argv[])
{
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

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = connfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
            }
            else if (events[i].events & EPOLLIN)
            {
                std::cout << "接收数据+++++++++++++++++++++++=" << std::endl;
                epoll_ret = recv(clientfd, epoll_buffer, BUFFER_LENGTH, 0);
                if (epoll_ret > 0)
                {

                    printf("recv: %s, n: %d\n", epoll_buffer, epoll_ret);
                    std::string httpRequest(reinterpret_cast<char *>(epoll_buffer), epoll_ret);
                    epoll_httpResponse = generateHttpResponse(httpRequest);

                    ev.events = EPOLLOUT;
                    ev.data.fd = clientfd;
                    epoll_ctl(epfd, EPOLL_CTL_MOD, clientfd, &ev);
                }
                else if (epoll_ret == 0)
                {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, clientfd, &ev);
                    close(clientfd);
                    std::cout << "关闭连接:" << clientfd << std::endl;
                }
            }
            else if (events[i].events & EPOLLOUT)
            {
                std::cout << "发送数据+++++++++++++++++++++++=" << std::endl;
                // int sent = send(clientfd, epoll_httpResponse.c_str(), epoll_httpResponse.length(), 0);

                int totalSent = 0;
                int dataLength = epoll_httpResponse.length();
                while (totalSent < dataLength)
                {
                    epoll_ret = send(clientfd, epoll_httpResponse.c_str() + totalSent, dataLength - totalSent, 0);
                    if (epoll_ret == -1)
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
                    totalSent += epoll_ret;
                }

                ev.events = EPOLLIN;
                ev.data.fd = clientfd;
                // epoll_ctl(epfd, EPOLL_CTL_MOD, clientfd, &ev);
                // 因为是Http所以处理完数据后关闭连接
                epoll_ctl(epfd, EPOLL_CTL_DEL, clientfd, &ev);
                close(clientfd);
                std::cout << "关闭连接:" << clientfd << std::endl;
            }
        }
    }
#endif
    return 0;
}
